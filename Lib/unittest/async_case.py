import asyncio
import inspect

from .case import TestCase



class AsyncioTestCase(TestCase):
    # Names intentionally have a long prefix
    # to reduce a chance of clashing with user-defined attributes
    # from inherited test case
    #
    # The class doesn't call loop.run_until_complete(self.setUp()) and family
    # but uses a different approach:
    # 1. create a long-running task that reads self.setUp()
    #    awaitable from queue along with a future
    # 2. await the awaitable object passing in and set the result
    #    into the future object
    # 3. Outer code puts the awaitable and the future object into a queue
    #    with waiting for the future
    # The trick is necessary because every run_until_complete() call
    # creates a new task with embedded ContextVar context.
    # To share contextvars between setUp(), test and tearDown() we need to execute
    # them inside the same task.

    def __init__(self, methodName='runTest'):
        super().__init__(methodName)
        self._asyncioTestLoop = None
        self._asyncioCallsQueue = None

        def _callSetUp(self):
            self._callMaybeAsync(self.setUp)

        def _callTearDown(self):
            self._callMaybeAsync(self.tearDown)

        def _callCleanup(self, function, *args, **kwargs):
            self._callMaybeAsync(function, *args, **kwargs)

        def _callMaybeAsync(self, func, *args, **kwargs):
            assert self._asyncioTestLoop is not None
            ret = func(*args, **kwargs)
            if inspect.isawaitable(ret):
                fut = self._asyncioTestLoop.create_future()
                self._asyncioCallsQueue.put_nowait(fut, ret)
                return self._asyncioTestLoop.run_until_complete(fut)
            else:
                return ret

        async def _asyncioLoopRunner(self):
            queue = self._asyncioCallsQueue
            while True:
                query = await queue.get()
                queue.task_done()
                if query is None:
                    return
                fut, awaitable = query
                try:
                    ret = await awaitable
                    if not fut.cancelled():
                        fut.set_result(ret)
                except asyncio.CancelledError:
                    raise
                except Exception as ex:
                    if not fut.cancelled():
                        fut.set_exception(ex)

        def _setupAsyncioLoop(self):
            if self._asyncioTestLoop is not None:
                return
            loop = asyncio.new_event_loop()
            asyncio.set_event_loop(loop)
            loop.set_debug(True)
            self._asyncioTestLoop = loop
            self._asyncioCallsQueue = asyncio.Queue(loop=loop)
            self._asyncioCallsTask = loop.create_task(self._asyncioLoopRunner())

        def _tearDownAsyncioLoop(self):
            if self._asyncioTestLoop is None:
                return
            loop = self._asyncioTestLoop
            self._asyncioTestLoop = None
            self._asyncioCallsQueue.put_nowait(None)
            loop.run_until_complete(self._asyncioCallsQueue.join())

            try:
                # cancel all tasks
                to_cancel = asyncio.all_tasks(loop)
                if not to_cancel:
                    return

                for task in to_cancel:
                    task.cancel()

                loop.run_until_complete(
                    asyncio.gather(*to_cancel, loop=loop, return_exceptions=True))

                for task in to_cancel:
                    if task.cancelled():
                        continue
                    if task.exception() is not None:
                        loop.call_exception_handler({
                            'message': 'unhandled exception during test shutdown',
                            'exception': task.exception(),
                            'task': task,
                        })
                # shutdown asyncgens
                loop.run_until_complete(loop.shutdown_asyncgens())
            finally:
                asyncio.set_event_loop(None)
                loop.close()

        def run(self, result=None):
            self._setupAsyncioLoop()
            try:
                return super().run(result)
            finally:
                self._tearDownAsyncioLoop()
