# -*-mode: python; fill-column: 75; tab-width: 8; coding: iso-latin-1-unix -*-
#
# $Id$
#
# Tix.py -- Tix widget wrappers.
#
#	For Tix, see http://tix.sourceforge.net
#
#       - Sudhir Shenoy (sshenoy@gol.com), Dec. 1995.
#         based on an idea of Jean-Marc Lugrin (lugrin@ms.com)
#
# NOTE: In order to minimize changes to Tkinter.py, some of the code here
#       (TixWidget.__init__) has been taken from Tkinter (Widget.__init__)
#       and will break if there are major changes in Tkinter.
#
# The Tix widgets are represented by a class hierarchy in python with proper
# inheritance of base classes.
#
# As a result after creating a 'w = StdButtonBox', I can write
#              w.ok['text'] = 'Who Cares'
#    or              w.ok['bg'] = w['bg']
# or even       w.ok.invoke()
# etc.
#
# Compare the demo tixwidgets.py to the original Tcl program and you will
# appreciate the advantages.
#

import string
from Tkinter import *
from Tkinter import _flatten, _cnfmerge, _default_root

# WARNING - TkVersion is a limited precision floating point number
if TkVersion < 3.999:
    raise ImportError, "This version of Tix.py requires Tk 4.0 or higher"

import _tkinter # If this fails your Python may not be configured for Tk
# TixVersion = string.atof(tkinter.TIX_VERSION) # If this fails your Python may not be configured for Tix
# WARNING - TixVersion is a limited precision floating point number

# Some more constants (for consistency with Tkinter)
WINDOW = 'window'
TEXT = 'text'
STATUS = 'status'
IMMEDIATE = 'immediate'
IMAGE = 'image'
IMAGETEXT = 'imagetext'
BALLOON = 'balloon'
AUTO = 'auto'
ACROSSTOP = 'acrosstop'

# Some constants used by Tkinter dooneevent()
TCL_DONT_WAIT     = 1 << 1
TCL_WINDOW_EVENTS = 1 << 2
TCL_FILE_EVENTS   = 1 << 3
TCL_TIMER_EVENTS  = 1 << 4
TCL_IDLE_EVENTS   = 1 << 5
TCL_ALL_EVENTS    = 0

# BEWARE - this is implemented by copying some code from the Widget class
#          in Tkinter (to override Widget initialization) and is therefore
#          liable to break.
import Tkinter, os

# Could probably add this to Tkinter.Misc
class tixCommand:
    """The tix commands provide access to miscellaneous  elements
    of  Tix's  internal state and the Tix application context.
    Most of the information manipulated by these  commands pertains
    to  the  application  as a whole, or to a screen or
    display, rather than to a particular window.

    This is a mixin class, assumed to be mixed to Tkinter.Tk
    that supports the self.tk.call method.
    """

    def tix_addbitmapdir(self, directory):
        """Tix maintains a list of directories under which
        the  tix_getimage  and tix_getbitmap commands will
        search for image files. The standard bitmap  directory
        is $TIX_LIBRARY/bitmaps. The addbitmapdir command
        adds directory into this list. By  using  this
        command, the  image  files  of an applications can
        also be located using the tix_getimage or tix_getbitmap
        command.
        """
        return self.tk.call('tix', 'addbitmapdir', directory)

    def tix_cget(self, option):
        """Returns  the  current  value  of the configuration
        option given by option. Option may be  any  of  the
        options described in the CONFIGURATION OPTIONS section.
        """
        return self.tk.call('tix', 'cget', option)

    def tix_configure(self, cnf=None, **kw):
        """Query or modify the configuration options of the Tix application
        context. If no option is specified, returns a dictionary all of the
        available options.  If option is specified with no value, then the
        command returns a list describing the one named option (this list
        will be identical to the corresponding sublist of the value
        returned if no option is specified).  If one or more option-value
        pairs are specified, then the command modifies the given option(s)
        to have the given value(s); in this case the command returns an
        empty string. Option may be any of the configuration options.
        """
        # Copied from Tkinter.py
        if kw:
            cnf = _cnfmerge((cnf, kw))
        elif cnf:
            cnf = _cnfmerge(cnf)
        if cnf is None:
            cnf = {}
            for x in self.tk.split(self.tk.call('tix', 'configure')):
                cnf[x[0][1:]] = (x[0][1:],) + x[1:]
            return cnf
        if isinstance(cnf, StringType):
            x = self.tk.split(self.tk.call('tix', 'configure', '-'+cnf))
            return (x[0][1:],) + x[1:]
        return self.tk.call(('tix', 'configure') + self._options(cnf))

    def tix_filedialog(self, dlgclass=None):
        """Returns the file selection dialog that may be shared among
        different calls from this application.  This command will create a
        file selection dialog widget when it is called the first time. This
        dialog will be returned by all subsequent calls to tix_filedialog.
        An optional dlgclass parameter can be passed to specified what type
        of file selection dialog widget is desired. Possible options are
        tix FileSelectDialog or tixExFileSelectDialog.
        """
        if dlgclass is not None:
            return self.tk.call('tix', 'filedialog', dlgclass)
        else:
            return self.tk.call('tix', 'filedialog')

    def tix_getbitmap(self, name):
        """Locates a bitmap file of the name name.xpm or name in one of the
        bitmap directories (see the tix_addbitmapdir command above).  By
        using tix_getbitmap, you can avoid hard coding the pathnames of the
        bitmap files in your application. When successful, it returns the
        complete pathname of the bitmap file, prefixed with the character
        '@'.  The returned value can be used to configure the -bitmap
        option of the TK and Tix widgets.
        """
        return self.tk.call('tix', 'getbitmap', name)

    def tix_getimage(self, name):
        """Locates an image file of the name name.xpm, name.xbm or name.ppm
        in one of the bitmap directories (see the addbitmapdir command
        above). If more than one file with the same name (but different
        extensions) exist, then the image type is chosen according to the
        depth of the X display: xbm images are chosen on monochrome
        displays and color images are chosen on color displays. By using
        tix_ getimage, you can advoid hard coding the pathnames of the
        image files in your application. When successful, this command
        returns the name of the newly created image, which can be used to
        configure the -image option of the Tk and Tix widgets.
        """
        return self.tk.call('tix', 'getimage', name)

    def tix_option_get(self, name):
        """Gets  the options  manitained  by  the  Tix
        scheme mechanism. Available options include:

            active_bg       active_fg      bg
            bold_font       dark1_bg       dark1_fg
            dark2_bg        dark2_fg       disabled_fg
            fg       	    fixed_font     font
            inactive_bg     inactive_fg    input1_bg
            input2_bg       italic_font    light1_bg
            light1_fg       light2_bg      light2_fg
            menu_font       output1_bg     output2_bg
            select_bg       select_fg      selector
            """
        # could use self.tk.globalgetvar('tixOption', name)
        return self.tk.call('tix', 'option', 'get', name)

    def tix_resetoptions(self, newScheme, newFontSet, newScmPrio=None):
        """Resets the scheme and fontset of the Tix application to
        newScheme and newFontSet, respectively.  This affects only those
        widgets created after this call. Therefore, it is best to call the
        resetoptions command before the creation of any widgets in a Tix
        application.

        The optional parameter newScmPrio can be given to reset the
        priority level of the Tk options set by the Tix schemes.

        Because of the way Tk handles the X option database, after Tix has
        been has imported and inited, it is not possible to reset the color
        schemes and font sets using the tix config command.  Instead, the
        tix_resetoptions command must be used.
        """
        if newScmPrio is not None:
            return self.tk.call('tix', 'resetoptions', newScheme, newFontSet, newScmPrio)
        else:
            return self.tk.call('tix', 'resetoptions', newScheme, newFontSet)

class Tk(Tkinter.Tk, tixCommand):
    """Toplevel widget of Tix which represents mostly the main window
    of an application. It has an associated Tcl interpreter."""
    def __init__(self, screenName=None, baseName=None, className='Tix'):
        Tkinter.Tk.__init__(self, screenName, baseName, className)
        tixlib = os.environ.get('TIX_LIBRARY')
        self.tk.eval('global auto_path; lappend auto_path [file dir [info nameof]]')
        if tixlib is not None:
            self.tk.eval('global auto_path; lappend auto_path {%s}' % tixlib)
            self.tk.eval('global tcl_pkgPath; lappend tcl_pkgPath {%s}' % tixlib)
        # Load Tix - this should work dynamically or statically
        # If it's static, lib/tix8.1/pkgIndex.tcl should have
        #		'load {} Tix'
        # If it's dynamic under Unix, lib/tix8.1/pkgIndex.tcl should have
        #		'load libtix8.1.8.3.so Tix'
        self.tk.eval('package require Tix')


# The Tix 'tixForm' geometry manager
class Form:
    """The Tix Form geometry manager

    Widgets can be arranged by specifying attachments to other widgets.
    See Tix documentation for complete details"""

    def config(self, cnf={}, **kw):
        apply(self.tk.call, ('tixForm', self._w) + self._options(cnf, kw))

    form = config

    def __setitem__(self, key, value):
        Form.form(self, {key: value})

    def check(self):
        return self.tk.call('tixForm', 'check', self._w)

    def forget(self):
        self.tk.call('tixForm', 'forget', self._w)

    def grid(self, xsize=0, ysize=0):
        if (not xsize) and (not ysize):
            x = self.tk.call('tixForm', 'grid', self._w)
            y = self.tk.splitlist(x)
            z = ()
            for x in y:
                z = z + (self.tk.getint(x),)
            return z
        self.tk.call('tixForm', 'grid', self._w, xsize, ysize)

    def info(self, option=None):
        if not option:
            return self.tk.call('tixForm', 'info', self._w)
        if option[0] != '-':
            option = '-' + option
        return self.tk.call('tixForm', 'info', self._w, option)

    def slaves(self):
        return map(self._nametowidget,
                   self.tk.splitlist(
                       self.tk.call(
                       'tixForm', 'slaves', self._w)))


    

Tkinter.Widget.__bases__ = Tkinter.Widget.__bases__ + (Form,)

class TixWidget(Tkinter.Widget):
    """A TixWidget class is used to package all (or most) Tix widgets.

    Widget initialization is extended in two ways:
       1) It is possible to give a list of options which must be part of
       the creation command (so called Tix 'static' options). These cannot be
       given as a 'config' command later.
       2) It is possible to give the name of an existing TK widget. These are
       child widgets created automatically by a Tix mega-widget. The Tk call
       to create these widgets is therefore bypassed in TixWidget.__init__

    Both options are for use by subclasses only.
    """
    def __init__ (self, master=None, widgetName=None,
                static_options=None, cnf={}, kw={}):
       # Merge keywords and dictionary arguments
       if kw:
            cnf = _cnfmerge((cnf, kw))
       else:
           cnf = _cnfmerge(cnf)

       # Move static options into extra. static_options must be
       # a list of keywords (or None).
       extra=()
       if static_options:
           for k,v in cnf.items()[:]:
              if k in static_options:
                  extra = extra + ('-' + k, v)
                  del cnf[k]

       self.widgetName = widgetName
       Widget._setup(self, master, cnf)

       # If widgetName is None, this is a dummy creation call where the
       # corresponding Tk widget has already been created by Tix
       if widgetName:
           apply(self.tk.call, (widgetName, self._w) + extra)

       # Non-static options - to be done via a 'config' command
       if cnf:
           Widget.config(self, cnf)

       # Dictionary to hold subwidget names for easier access. We can't
       # use the children list because the public Tix names may not be the
       # same as the pathname component
       self.subwidget_list = {}

    # We set up an attribute access function so that it is possible to
    # do w.ok['text'] = 'Hello' rather than w.subwidget('ok')['text'] = 'Hello'
    # when w is a StdButtonBox.
    # We can even do w.ok.invoke() because w.ok is subclassed from the
    # Button class if you go through the proper constructors
    def __getattr__(self, name):
       if self.subwidget_list.has_key(name):
           return self.subwidget_list[name]
       raise AttributeError, name

    def set_silent(self, value):
       """Set a variable without calling its action routine"""
       self.tk.call('tixSetSilent', self._w, value)

    def subwidget(self, name):
       """Return the named subwidget (which must have been created by
       the sub-class)."""
       n = self._subwidget_name(name)
       if not n:
           raise TclError, "Subwidget " + name + " not child of " + self._name
       # Remove header of name and leading dot
       n = n[len(self._w)+1:]
       return self._nametowidget(n)

    def subwidgets_all(self):
       """Return all subwidgets."""
       names = self._subwidget_names()
       if not names:
           return []
       retlist = []
       for name in names:
           name = name[len(self._w)+1:]
           try:
              retlist.append(self._nametowidget(name))
           except:
              # some of the widgets are unknown e.g. border in LabelFrame
              pass
       return retlist

    def _subwidget_name(self,name):
       """Get a subwidget name (returns a String, not a Widget !)"""
       try:
           return self.tk.call(self._w, 'subwidget', name)
       except TclError:
           return None

    def _subwidget_names(self):
       """Return the name of all subwidgets."""
       try:
           x = self.tk.call(self._w, 'subwidgets', '-all')
           return self.tk.split(x)
       except TclError:
           return None

    def config_all(self, option, value):
       """Set configuration options for all subwidgets (and self)."""
       if option == '':
           return
       elif not isinstance(option, StringType):
           option = `option`
       if not isinstance(value, StringType):
           value = `value`
       names = self._subwidget_names()
       for name in names:
           self.tk.call(name, 'configure', '-' + option, value)

# Subwidgets are child widgets created automatically by mega-widgets.
# In python, we have to create these subwidgets manually to mirror their
# existence in Tk/Tix.
class TixSubWidget(TixWidget):
    """Subwidget class.

    This is used to mirror child widgets automatically created
    by Tix/Tk as part of a mega-widget in Python (which is not informed
    of this)"""

    def __init__(self, master, name,
               destroy_physically=1, check_intermediate=1):
       if check_intermediate:
           path = master._subwidget_name(name)
           try:
              path = path[len(master._w)+1:]
              plist = string.splitfields(path, '.')
           except:
              plist = []

       if (not check_intermediate) or len(plist) < 2:
           # immediate descendant
           TixWidget.__init__(self, master, None, None, {'name' : name})
       else:
           # Ensure that the intermediate widgets exist
           parent = master
           for i in range(len(plist) - 1):
              n = string.joinfields(plist[:i+1], '.')
              try:
                  w = master._nametowidget(n)
                  parent = w
              except KeyError:
                  # Create the intermediate widget
                  parent = TixSubWidget(parent, plist[i],
                                     destroy_physically=0,
                                     check_intermediate=0)
           TixWidget.__init__(self, parent, None, None, {'name' : name})
       self.destroy_physically = destroy_physically

    def destroy(self):
       # For some widgets e.g., a NoteBook, when we call destructors,
       # we must be careful not to destroy the frame widget since this
       # also destroys the parent NoteBook thus leading to an exception
       # in Tkinter when it finally calls Tcl to destroy the NoteBook
       for c in self.children.values(): c.destroy()
       if self.master.children.has_key(self._name):
           del self.master.children[self._name]
       if self.master.subwidget_list.has_key(self._name):
           del self.master.subwidget_list[self._name]
       if self.destroy_physically:
           # This is bypassed only for a few widgets
           self.tk.call('destroy', self._w)


# Useful func. to split Tcl lists and return as a dict. From Tkinter.py
def _lst2dict(lst):
    dict = {}
    for x in lst:
       dict[x[0][1:]] = (x[0][1:],) + x[1:]
    return dict

# Useful class to create a display style - later shared by many items.
# Contributed by Steffen Kremser
class DisplayStyle:
    """DisplayStyle - handle configuration options shared by
    (multiple) Display Items"""

    def __init__(self, itemtype, cnf={}, **kw ):
        master = _default_root              # global from Tkinter
        if not master and cnf.has_key('refwindow'): master=cnf['refwindow']
        elif not master and kw.has_key('refwindow'):  master= kw['refwindow']
        elif not master: raise RuntimeError, "Too early to create display style: no root window"
        self.tk = master.tk
        self.stylename = apply(self.tk.call, ('tixDisplayStyle', itemtype) +
                            self._options(cnf,kw) )

    def __str__(self):
       return self.stylename
 
    def _options(self, cnf, kw ):
       if kw and cnf:
           cnf = _cnfmerge((cnf, kw))
       elif kw:
           cnf = kw
       opts = ()
       for k, v in cnf.items():
           opts = opts + ('-'+k, v)
       return opts
 
    def delete(self):
       self.tk.call(self.stylename, 'delete')
 
    def __setitem__(self,key,value):
       self.tk.call(self.stylename, 'configure', '-%s'%key, value)
 
    def config(self, cnf={}, **kw):
       return _lst2dict(
           self.tk.split(
              apply(self.tk.call,
                    (self.stylename, 'configure') + self._options(cnf,kw))))
 
    def __getitem__(self,key):
       return self.tk.call(self.stylename, 'cget', '-%s'%key)


######################################################
### The Tix Widget classes - in alphabetical order ###
######################################################

class Balloon(TixWidget):
    """Balloon help widget.

    Subwidget       Class
    ---------       -----
    label           Label
    message         Message"""

    def __init__(self, master=None, cnf={}, **kw):
        # static seem to be -installcolormap -initwait -statusbar -cursor
       static = ['options', 'installcolormap', 'initwait', 'statusbar',
                 'cursor']
       TixWidget.__init__(self, master, 'tixBalloon', static, cnf, kw)
       self.subwidget_list['label'] = _dummyLabel(self, 'label',
                                                  destroy_physically=0)
       self.subwidget_list['message'] = _dummyLabel(self, 'message',
                                                    destroy_physically=0)

    def bind_widget(self, widget, cnf={}, **kw):
       """Bind balloon widget to another.
       One balloon widget may be bound to several widgets at the same time"""
       apply(self.tk.call, 
             (self._w, 'bind', widget._w) + self._options(cnf, kw))

    def unbind_widget(self, widget):
       self.tk.call(self._w, 'unbind', widget._w)

class ButtonBox(TixWidget):
    """ButtonBox - A container for pushbuttons.
    Subwidgets are the buttons added with the add method.
    """
    def __init__(self, master=None, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixButtonBox',
                          ['orientation', 'options'], cnf, kw)

    def add(self, name, cnf={}, **kw):
       """Add a button with given name to box."""

       btn = apply(self.tk.call,
                   (self._w, 'add', name) + self._options(cnf, kw))
       self.subwidget_list[name] = _dummyButton(self, name)
       return btn

    def invoke(self, name):
       if self.subwidget_list.has_key(name):
           self.tk.call(self._w, 'invoke', name)

class ComboBox(TixWidget):
    """ComboBox - an Entry field with a dropdown menu. The user can select a
    choice by either typing in the entry subwdget or selecting from the
    listbox subwidget.

    Subwidget       Class
    ---------       -----
    entry       Entry
    arrow       Button
    slistbox    ScrolledListBox
    tick        Button 
    cross       Button : present if created with the fancy option"""

    def __init__ (self, master=None, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixComboBox', 
                          ['editable', 'dropdown', 'fancy', 'options'],
                          cnf, kw)
       self.subwidget_list['label'] = _dummyLabel(self, 'label')
       self.subwidget_list['entry'] = _dummyEntry(self, 'entry')
       self.subwidget_list['arrow'] = _dummyButton(self, 'arrow')
       self.subwidget_list['slistbox'] = _dummyScrolledListBox(self,
                                                               'slistbox')
       try:
           self.subwidget_list['tick'] = _dummyButton(self, 'tick')
           self.subwidget_list['cross'] = _dummyButton(self, 'cross')
       except TypeError:
           # unavailable when -fancy not specified
           pass

    def add_history(self, str):
       self.tk.call(self._w, 'addhistory', str)

    def append_history(self, str):
       self.tk.call(self._w, 'appendhistory', str)

    def insert(self, index, str):
       self.tk.call(self._w, 'insert', index, str)

    def pick(self, index):
       self.tk.call(self._w, 'pick', index)

class Control(TixWidget):
    """Control - An entry field with value change arrows.  The user can
    adjust the value by pressing the two arrow buttons or by entering
    the value directly into the entry. The new value will be checked
    against the user-defined upper and lower limits.

    Subwidget       Class
    ---------       -----
    incr       Button
    decr       Button
    entry       Entry
    label       Label"""

    def __init__ (self, master=None, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixControl', ['options'], cnf, kw)
       self.subwidget_list['incr'] = _dummyButton(self, 'incr')
       self.subwidget_list['decr'] = _dummyButton(self, 'decr')
       self.subwidget_list['label'] = _dummyLabel(self, 'label')
       self.subwidget_list['entry'] = _dummyEntry(self, 'entry')

    def decrement(self):
       self.tk.call(self._w, 'decr')

    def increment(self):
       self.tk.call(self._w, 'incr')

    def invoke(self):
       self.tk.call(self._w, 'invoke')

    def update(self):
       self.tk.call(self._w, 'update')

class DirList(TixWidget):
    """DirList - displays a list view of a directory, its previous
    directories and its sub-directories. The user can choose one of
    the directories displayed in the list or change to another directory.

    Subwidget       Class
    ---------       -----
    hlist       HList
    hsb              Scrollbar
    vsb              Scrollbar"""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixDirList', ['options'], cnf, kw)
       self.subwidget_list['hlist'] = _dummyHList(self, 'hlist')
       self.subwidget_list['vsb'] = _dummyScrollbar(self, 'vsb')
       self.subwidget_list['hsb'] = _dummyScrollbar(self, 'hsb')

    def chdir(self, dir):
       self.tk.call(self._w, 'chdir', dir)

class DirTree(TixWidget):
    """DirTree - Directory Listing in a hierarchical view.
    Displays a tree view of a directory, its previous directories and its
    sub-directories. The user can choose one of the directories displayed
    in the list or change to another directory.

    Subwidget       Class
    ---------       -----
    hlist       HList
    hsb              Scrollbar
    vsb              Scrollbar"""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixDirTree', ['options'], cnf, kw)
       self.subwidget_list['hlist'] = _dummyHList(self, 'hlist')
       self.subwidget_list['vsb'] = _dummyScrollbar(self, 'vsb')
       self.subwidget_list['hsb'] = _dummyScrollbar(self, 'hsb')

    def chdir(self, dir):
       self.tk.call(self._w, 'chdir', dir)

class DirSelectBox(TixWidget):
    """DirSelectBox - Motif style file select box.
    It is generally used for
    the user to choose a file. FileSelectBox stores the files mostly
    recently selected into a ComboBox widget so that they can be quickly
    selected again.
    
    Subwidget       Class
    ---------       -----
    selection       ComboBox
    filter       ComboBox
    dirlist       ScrolledListBox
    filelist       ScrolledListBox"""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixDirSelectBox', ['options'], cnf, kw)
       self.subwidget_list['dirlist'] = _dummyDirList(self, 'dirlist')
       self.subwidget_list['dircbx'] = _dummyFileComboBox(self, 'dircbx')

class ExFileSelectBox(TixWidget):
    """ExFileSelectBox - MS Windows style file select box.
    It provides an convenient method for the user to select files.

    Subwidget       Class
    ---------       -----
    cancel       Button
    ok              Button
    hidden       Checkbutton
    types       ComboBox
    dir              ComboBox
    file       ComboBox
    dirlist       ScrolledListBox
    filelist       ScrolledListBox"""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixExFileSelectBox', ['options'], cnf, kw)
       self.subwidget_list['cancel'] = _dummyButton(self, 'cancel')
       self.subwidget_list['ok'] = _dummyButton(self, 'ok')
       self.subwidget_list['hidden'] = _dummyCheckbutton(self, 'hidden')
       self.subwidget_list['types'] = _dummyComboBox(self, 'types')
       self.subwidget_list['dir'] = _dummyComboBox(self, 'dir')
       self.subwidget_list['dirlist'] = _dummyDirList(self, 'dirlist')
       self.subwidget_list['file'] = _dummyComboBox(self, 'file')
       self.subwidget_list['filelist'] = _dummyScrolledListBox(self, 'filelist')

    def filter(self):
       self.tk.call(self._w, 'filter')

    def invoke(self):
       self.tk.call(self._w, 'invoke')


# Should inherit from a Dialog class
class DirSelectDialog(TixWidget):
    """The DirSelectDialog widget presents the directories in the file
    system in a dialog window. The user can use this dialog window to
    navigate through the file system to select the desired directory.

    Subwidgets       Class
    ----------       -----
    dirbox       DirSelectDialog"""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixDirSelectDialog',
                        ['options'], cnf, kw)
       self.subwidget_list['dirbox'] = _dummyDirSelectBox(self, 'dirbox')
       # cancel and ok buttons are missing
       
    def popup(self):
       self.tk.call(self._w, 'popup')

    def popdown(self):
       self.tk.call(self._w, 'popdown')


# Should inherit from a Dialog class
class ExFileSelectDialog(TixWidget):
    """ExFileSelectDialog - MS Windows style file select dialog.
    It provides an convenient method for the user to select files.

    Subwidgets       Class
    ----------       -----
    fsbox       ExFileSelectBox"""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixExFileSelectDialog',
                        ['options'], cnf, kw)
       self.subwidget_list['fsbox'] = _dummyExFileSelectBox(self, 'fsbox')

    def popup(self):
       self.tk.call(self._w, 'popup')

    def popdown(self):
       self.tk.call(self._w, 'popdown')

class FileSelectBox(TixWidget):
    """ExFileSelectBox - Motif style file select box.
    It is generally used for
    the user to choose a file. FileSelectBox stores the files mostly
    recently selected into a ComboBox widget so that they can be quickly
    selected again.
    
    Subwidget       Class
    ---------       -----
    selection       ComboBox
    filter       ComboBox
    dirlist       ScrolledListBox
    filelist       ScrolledListBox"""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixFileSelectBox', ['options'], cnf, kw)
       self.subwidget_list['dirlist'] = _dummyScrolledListBox(self, 'dirlist')
       self.subwidget_list['filelist'] = _dummyScrolledListBox(self, 'filelist')
       self.subwidget_list['filter'] = _dummyComboBox(self, 'filter')
       self.subwidget_list['selection'] = _dummyComboBox(self, 'selection')

    def apply_filter(self):              # name of subwidget is same as command
       self.tk.call(self._w, 'filter')

    def invoke(self):
       self.tk.call(self._w, 'invoke')

# Should inherit from a Dialog class
class FileSelectDialog(TixWidget):
    """FileSelectDialog - Motif style file select dialog.

    Subwidgets       Class
    ----------       -----
    btns       StdButtonBox
    fsbox       FileSelectBox"""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixFileSelectDialog',
                        ['options'], cnf, kw)
       self.subwidget_list['btns'] = _dummyStdButtonBox(self, 'btns')
       self.subwidget_list['fsbox'] = _dummyFileSelectBox(self, 'fsbox')

    def popup(self):
       self.tk.call(self._w, 'popup')

    def popdown(self):
       self.tk.call(self._w, 'popdown')

class FileEntry(TixWidget):
    """FileEntry - Entry field with button that invokes a FileSelectDialog.
    The user can type in the filename manually. Alternatively, the user can
    press the button widget that sits next to the entry, which will bring
    up a file selection dialog.

    Subwidgets       Class
    ----------       -----
    button       Button
    entry       Entry"""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixFileEntry',
                        ['dialogtype', 'options'], cnf, kw)
       self.subwidget_list['button'] = _dummyButton(self, 'button')
       self.subwidget_list['entry'] = _dummyEntry(self, 'entry')

    def invoke(self):
       self.tk.call(self._w, 'invoke')

    def file_dialog(self):
       # XXX return python object
       pass

class HList(TixWidget):
    """HList - Hierarchy display  widget can be used to display any data
    that have a hierarchical structure, for example, file system directory
    trees. The list entries are indented and connected by branch lines
    according to their places in the hierachy.

    Subwidgets - None"""

    def __init__ (self,master=None,cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixHList',
                        ['columns', 'options'], cnf, kw)

    def add(self, entry, cnf={}, **kw):
       return apply(self.tk.call,
                   (self._w, 'add', entry) + self._options(cnf, kw))

    def add_child(self, parent=None, cnf={}, **kw):
       if not parent:
           parent = ''
       return apply(self.tk.call,
                   (self._w, 'addchild', parent) + self._options(cnf, kw))

    def anchor_set(self, entry):
       self.tk.call(self._w, 'anchor', 'set', entry)

    def anchor_clear(self):
       self.tk.call(self._w, 'anchor', 'clear')

    def column_width(self, col=0, width=None, chars=None):
       if not chars:
           return self.tk.call(self._w, 'column', 'width', col, width)
       else:
           return self.tk.call(self._w, 'column', 'width', col,
                            '-char', chars)

    def delete_all(self):
       self.tk.call(self._w, 'delete', 'all')

    def delete_entry(self, entry):
       self.tk.call(self._w, 'delete', 'entry', entry)

    def delete_offsprings(self, entry):
       self.tk.call(self._w, 'delete', 'offsprings', entry)

    def delete_siblings(self, entry):
       self.tk.call(self._w, 'delete', 'siblings', entry)

    def dragsite_set(self, index):
       self.tk.call(self._w, 'dragsite', 'set', index)

    def dragsite_clear(self):
       self.tk.call(self._w, 'dragsite', 'clear')

    def dropsite_set(self, index):
       self.tk.call(self._w, 'dropsite', 'set', index)

    def dropsite_clear(self):
       self.tk.call(self._w, 'dropsite', 'clear')

    def header_create(self, col, cnf={}, **kw):
        apply(self.tk.call,
              (self._w, 'header', 'create', col) + self._options(cnf, kw))
 
    def header_configure(self, col, cnf={}, **kw):
       if cnf is None:
           return _lst2dict(
              self.tk.split(
                  self.tk.call(self._w, 'header', 'configure', col)))
       apply(self.tk.call, (self._w, 'header', 'configure', col)
             + self._options(cnf, kw))
 
    def header_cget(self,  col, opt):
       return self.tk.call(self._w, 'header', 'cget', col, opt)
 
    def header_exists(self,  col):
       return self.tk.call(self._w, 'header', 'exists', col)
 
    def header_delete(self, col):
        self.tk.call(self._w, 'header', 'delete', col)
 
    def header_size(self, col):
        return self.tk.call(self._w, 'header', 'size', col)
 
    def hide_entry(self, entry):
       self.tk.call(self._w, 'hide', 'entry', entry)

    def indicator_create(self, entry, cnf={}, **kw):
        apply(self.tk.call,
              (self._w, 'indicator', 'create', entry) + self._options(cnf, kw))
 
    def indicator_configure(self, entry, cnf={}, **kw):
       if cnf is None:
           return _lst2dict(
              self.tk.split(
                  self.tk.call(self._w, 'indicator', 'configure', entry)))
       apply(self.tk.call,
             (self._w, 'indicator', 'configure', entry) + self._options(cnf, kw))
 
    def indicator_cget(self,  entry, opt):
       return self.tk.call(self._w, 'indicator', 'cget', entry, opt)
 
    def indicator_exists(self,  entry):
       return self.tk.call (self._w, 'indicator', 'exists', entry)
 
    def indicator_delete(self, entry):
        self.tk.call(self._w, 'indicator', 'delete', entry)
 
    def indicator_size(self, entry):
        return self.tk.call(self._w, 'indicator', 'size', entry)

    def info_anchor(self):
       return self.tk.call(self._w, 'info', 'anchor')

    def info_children(self, entry=None):
       c = self.tk.call(self._w, 'info', 'children', entry)
       return self.tk.splitlist(c)

    def info_data(self, entry):
       return self.tk.call(self._w, 'info', 'data', entry)

    def info_exists(self, entry):
       return self.tk.call(self._w, 'info', 'exists', entry)

    def info_hidden(self, entry):
       return self.tk.call(self._w, 'info', 'hidden', entry)

    def info_next(self, entry):
       return self.tk.call(self._w, 'info', 'next', entry)

    def info_parent(self, entry):
       return self.tk.call(self._w, 'info', 'parent', entry)

    def info_prev(self, entry):
       return self.tk.call(self._w, 'info', 'prev', entry)

    def info_selection(self):
       c = self.tk.call(self._w, 'info', 'selection')
       return self.tk.splitlist(c)

    def item_cget(self, entry, col, opt):
       return self.tk.call(self._w, 'item', 'cget', entry, col, opt)
 
    def item_configure(self, entry, col, cnf={}, **kw):
       if cnf is None:
           return _lst2dict(
              self.tk.split(
                  self.tk.call(self._w, 'item', 'configure', entry, col)))
       apply(self.tk.call, (self._w, 'item', 'configure', entry, col) +
             self._options(cnf, kw))

    def item_create(self, entry, col, cnf={}, **kw):
       apply(self.tk.call,
             (self._w, 'item', 'create', entry, col) + self._options(cnf, kw))

    def item_exists(self, entry, col):
       return self.tk.call(self._w, 'item', 'exists', entry, col)
 
    def item_delete(self, entry, col):
       self.tk.call(self._w, 'item', 'delete', entry, col)

    def nearest(self, y):
       return self.tk.call(self._w, 'nearest', y)

    def see(self, entry):
       self.tk.call(self._w, 'see', entry)

    def selection_clear(self, cnf={}, **kw):
       apply(self.tk.call,
             (self._w, 'selection', 'clear') + self._options(cnf, kw))

    def selection_includes(self, entry):
       return self.tk.call(self._w, 'selection', 'includes', entry)

    def selection_set(self, first, last=None):
       self.tk.call(self._w, 'selection', 'set', first, last)

    def show_entry(self, entry):
       return self.tk.call(self._w, 'show', 'entry', entry)

    def xview(self, *args):
       apply(self.tk.call, (self._w, 'xview') + args)

    def yview(self, *args):
       apply(self.tk.call, (self._w, 'yview') + args)

class InputOnly(TixWidget):
    """InputOnly - Invisible widget.

    Subwidgets - None"""

    def __init__ (self,master=None,cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixInputOnly', None, cnf, kw)

class LabelEntry(TixWidget):
    """LabelEntry - Entry field with label. Packages an entry widget
    and a label into one mega widget. It can beused be used to simplify
    the creation of ``entry-form'' type of interface.

    Subwidgets       Class
    ----------       -----
    label       Label
    entry       Entry"""

    def __init__ (self,master=None,cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixLabelEntry',
                        ['labelside','options'], cnf, kw)
       self.subwidget_list['label'] = _dummyLabel(self, 'label')
       self.subwidget_list['entry'] = _dummyEntry(self, 'entry')

class LabelFrame(TixWidget):
    """LabelFrame - Labelled Frame container. Packages a frame widget
    and a label into one mega widget. To create widgets inside a
    LabelFrame widget, one creates the new widgets relative to the
    frame subwidget and manage them inside the frame subwidget.

    Subwidgets       Class
    ----------       -----
    label       Label
    frame       Frame"""

    def __init__ (self,master=None,cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixLabelFrame',
                        ['labelside','options'], cnf, kw)
       self.subwidget_list['label'] = _dummyLabel(self, 'label')
       self.subwidget_list['frame'] = _dummyFrame(self, 'frame')


class ListNoteBook(TixWidget):
    """A ListNoteBook widget is very similar to the TixNoteBook widget:
    it can be used to display many windows in a limited space using a
    notebook metaphor. The notebook is divided into a stack of pages
    (windows). At one time only one of these pages can be shown.
    The user can navigate through these pages by
    choosing the name of the desired page in the hlist subwidget."""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixDirList', ['options'], cnf, kw)
       self.subwidget_list['hlist'] = _dummyHList(self, 'hlist')
       self.subwidget_list['shlist'] = _dummyScrolledHList(self, 'vsb')


    def add(self, name, cnf={}, **kw):
       apply(self.tk.call,
             (self._w, 'add', name) + self._options(cnf, kw))
       self.subwidget_list[name] = TixSubWidget(self, name)
       return self.subwidget_list[name]

    def raise_page(self, name):              # raise is a python keyword
       self.tk.call(self._w, 'raise', name)

class Meter(TixWidget):
    """The Meter widget can be used to show the progress of a background
    job which may take a long time to execute.
    """

    def __init__(self, master=None, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixMeter',
                        ['options'], cnf, kw)

class NoteBook(TixWidget):
    """NoteBook - Multi-page container widget (tabbed notebook metaphor).

    Subwidgets       Class
    ----------       -----
    nbframe       NoteBookFrame
    <pages>       page widgets added dynamically with the add method"""

    def __init__ (self,master=None,cnf={}, **kw):
       TixWidget.__init__(self,master,'tixNoteBook', ['options'], cnf, kw)
       self.subwidget_list['nbframe'] = TixSubWidget(self, 'nbframe',
                                                destroy_physically=0)

    def add(self, name, cnf={}, **kw):
       apply(self.tk.call,
             (self._w, 'add', name) + self._options(cnf, kw))
       self.subwidget_list[name] = TixSubWidget(self, name)
       return self.subwidget_list[name]

    def delete(self, name):
       self.tk.call(self._w, 'delete', name)
       self.subwidget_list[name].destroy()
       del self.subwidget_list[name]

    def page(self, name):
       return self.subwidget(name)

    def pages(self):
       # Can't call subwidgets_all directly because we don't want .nbframe
       names = self.tk.split(self.tk.call(self._w, 'pages'))
       ret = []
       for x in names:
           ret.append(self.subwidget(x))
       return ret

    def raise_page(self, name):              # raise is a python keyword
       self.tk.call(self._w, 'raise', name)

    def raised(self):
       return self.tk.call(self._w, 'raised')

class NoteBookFrame(TixWidget):
    """Will be added when Tix documentation is available !!!"""
    pass

class OptionMenu(TixWidget):
    """OptionMenu - creates a menu button of options.

    Subwidget       Class
    ---------       -----
    menubutton       Menubutton
    menu       Menu"""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixOptionMenu', ['options'], cnf, kw)
       self.subwidget_list['menubutton'] = _dummyMenubutton(self, 'menubutton')
       self.subwidget_list['menu'] = _dummyMenu(self, 'menu')

    def add_command(self, name, cnf={}, **kw):
       apply(self.tk.call,
             (self._w, 'add', 'command', name) + self._options(cnf, kw))

    def add_separator(self, name, cnf={}, **kw):
       apply(self.tk.call,
             (self._w, 'add', 'separator', name) + self._options(cnf, kw))

    def delete(self, name):
       self.tk.call(self._w, 'delete', name)

    def disable(self, name):
       self.tk.call(self._w, 'disable', name)

    def enable(self, name):
       self.tk.call(self._w, 'enable', name)

class PanedWindow(TixWidget):
    """PanedWindow - Multi-pane container widget
    allows the user to interactively manipulate the sizes of several
    panes. The panes can be arranged either vertically or horizontally.The
    user changes the sizes of the panes by dragging the resize handle
    between two panes.

    Subwidgets       Class
    ----------       -----
    <panes>       g/p widgets added dynamically with the add method."""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixPanedWindow', ['orientation', 'options'], cnf, kw)

    def add(self, name, cnf={}, **kw):
       apply(self.tk.call,
             (self._w, 'add', name) + self._options(cnf, kw))
       self.subwidget_list[name] = TixSubWidget(self, name,
                                           check_intermediate=0)
       return self.subwidget_list[name]

    def panes(self):
       names = self.tk.call(self._w, 'panes')
       ret = []
       for x in names:
           ret.append(self.subwidget(x))
       return ret

class PopupMenu(TixWidget):
    """PopupMenu widget can be used as a replacement of the tk_popup command.
    The advantage of the Tix PopupMenu widget is it requires less application
    code to manipulate.


    Subwidgets       Class
    ----------       -----
    menubutton       Menubutton
    menu       Menu"""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixPopupMenu', ['options'], cnf, kw)
       self.subwidget_list['menubutton'] = _dummyMenubutton(self, 'menubutton')
       self.subwidget_list['menu'] = _dummyMenu(self, 'menu')

    def bind_widget(self, widget):
       self.tk.call(self._w, 'bind', widget._w)

    def unbind_widget(self, widget):
       self.tk.call(self._w, 'unbind', widget._w)

    def post_widget(self, widget, x, y):
       self.tk.call(self._w, 'post', widget._w, x, y)

class ResizeHandle(TixWidget):
    """Internal widget to draw resize handles on Scrolled widgets."""

    def __init__(self, master, cnf={}, **kw):
       # There seems to be a Tix bug rejecting the configure method
       # Let's try making the flags -static
       flags = ['options', 'command', 'cursorfg', 'cursorbg',
                'handlesize', 'hintcolor', 'hintwidth',
                'x', 'y']
       # In fact, x y height width are configurable
       TixWidget.__init__(self, master, 'tixResizeHandle',
                           flags, cnf, kw)

    def attach_widget(self, widget):
       self.tk.call(self._w, 'attachwidget', widget._w)

    def detach_widget(self, widget):
       self.tk.call(self._w, 'detachwidget', widget._w)

    def hide(self, widget):
       self.tk.call(self._w, 'hide', widget._w)

    def show(self, widget):
       self.tk.call(self._w, 'show', widget._w)

class ScrolledHList(TixWidget):
    """ScrolledHList - HList with automatic scrollbars."""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixScrolledHList', ['options'],
                        cnf, kw)
       self.subwidget_list['hlist'] = _dummyHList(self, 'hlist')
       self.subwidget_list['vsb'] = _dummyScrollbar(self, 'vsb')
       self.subwidget_list['hsb'] = _dummyScrollbar(self, 'hsb')

class ScrolledListBox(TixWidget):
    """ScrolledListBox - Listbox with automatic scrollbars."""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixScrolledListBox', ['options'], cnf, kw)
       self.subwidget_list['listbox'] = _dummyListbox(self, 'listbox')
       self.subwidget_list['vsb'] = _dummyScrollbar(self, 'vsb')
       self.subwidget_list['hsb'] = _dummyScrollbar(self, 'hsb')

class ScrolledText(TixWidget):
    """ScrolledText - Text with automatic scrollbars."""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixScrolledText', ['options'], cnf, kw)
       self.subwidget_list['text'] = _dummyText(self, 'text')
       self.subwidget_list['vsb'] = _dummyScrollbar(self, 'vsb')
       self.subwidget_list['hsb'] = _dummyScrollbar(self, 'hsb')

class ScrolledTList(TixWidget):
    """ScrolledTList - TList with automatic scrollbars."""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixScrolledTList', ['options'],
                        cnf, kw)
       self.subwidget_list['tlist'] = _dummyTList(self, 'tlist')
       self.subwidget_list['vsb'] = _dummyScrollbar(self, 'vsb')
       self.subwidget_list['hsb'] = _dummyScrollbar(self, 'hsb')

class ScrolledWindow(TixWidget):
    """ScrolledWindow - Window with automatic scrollbars."""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixScrolledWindow', ['options'], cnf, kw)
       self.subwidget_list['window'] = _dummyFrame(self, 'window')
       self.subwidget_list['vsb'] = _dummyScrollbar(self, 'vsb')
       self.subwidget_list['hsb'] = _dummyScrollbar(self, 'hsb')

class Select(TixWidget):
    """Select - Container of button subwidgets. It can be used to provide
    radio-box or check-box style of selection options for the user.

    Subwidgets are buttons added dynamically using the add method."""

    def __init__(self, master, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixSelect',
                        ['allowzero', 'radio', 'orientation', 'labelside',
                         'options'],
                        cnf, kw)
       self.subwidget_list['label'] = _dummyLabel(self, 'label')

    def add(self, name, cnf={}, **kw):
       apply(self.tk.call,
             (self._w, 'add', name) + self._options(cnf, kw))
       self.subwidget_list[name] = _dummyButton(self, name)
       return self.subwidget_list[name]

    def invoke(self, name):
       self.tk.call(self._w, 'invoke', name)

class StdButtonBox(TixWidget):
    """StdButtonBox - Standard Button Box (OK, Apply, Cancel and Help) """

    def __init__(self, master=None, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixStdButtonBox',
                        ['orientation', 'options'], cnf, kw)
       self.subwidget_list['ok'] = _dummyButton(self, 'ok')
       self.subwidget_list['apply'] = _dummyButton(self, 'apply')
       self.subwidget_list['cancel'] = _dummyButton(self, 'cancel')
       self.subwidget_list['help'] = _dummyButton(self, 'help')

    def invoke(self, name):
       if self.subwidget_list.has_key(name):
           self.tk.call(self._w, 'invoke', name)

class TList(TixWidget):
    """TList - Hierarchy display widget which can be
    used to display data in a tabular format. The list entries of a TList
    widget are similar to the entries in the Tk listbox widget. The main
    differences are (1) the TList widget can display the list entries in a
    two dimensional format and (2) you can use graphical images as well as
    multiple colors and fonts for the list entries.

    Subwidgets - None"""

    def __init__ (self,master=None,cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixTList', ['options'], cnf, kw)

    def active_set(self, index):
       self.tk.call(self._w, 'active', 'set', index)

    def active_clear(self):
       self.tk.call(self._w, 'active', 'clear')

    def anchor_set(self, index):
       self.tk.call(self._w, 'anchor', 'set', index)

    def anchor_clear(self):
       self.tk.call(self._w, 'anchor', 'clear')

    def delete(self, from_, to=None):
       self.tk.call(self._w, 'delete', from_, to)

    def dragsite_set(self, index):
       self.tk.call(self._w, 'dragsite', 'set', index)

    def dragsite_clear(self):
       self.tk.call(self._w, 'dragsite', 'clear')

    def dropsite_set(self, index):
       self.tk.call(self._w, 'dropsite', 'set', index)

    def dropsite_clear(self):
       self.tk.call(self._w, 'dropsite', 'clear')

    def insert(self, index, cnf={}, **kw):
       apply(self.tk.call,
              (self._w, 'insert', index) + self._options(cnf, kw))

    def info_active(self):
       return self.tk.call(self._w, 'info', 'active')

    def info_anchor(self):
       return self.tk.call(self._w, 'info', 'anchor')

    def info_down(self, index):
       return self.tk.call(self._w, 'info', 'down', index)

    def info_left(self, index):
       return self.tk.call(self._w, 'info', 'left', index)

    def info_right(self, index):
       return self.tk.call(self._w, 'info', 'right', index)

    def info_selection(self):
       c = self.tk.call(self._w, 'info', 'selection')
       return self.tk.splitlist(c)

    def info_size(self):
       return self.tk.call(self._w, 'info', 'size')

    def info_up(self, index):
       return self.tk.call(self._w, 'info', 'up', index)

    def nearest(self, x, y):
       return self.tk.call(self._w, 'nearest', x, y)

    def see(self, index):
       self.tk.call(self._w, 'see', index)

    def selection_clear(self, cnf={}, **kw):
       apply(self.tk.call,
             (self._w, 'selection', 'clear') + self._options(cnf, kw))

    def selection_includes(self, index):
       return self.tk.call(self._w, 'selection', 'includes', index)

    def selection_set(self, first, last=None):
       self.tk.call(self._w, 'selection', 'set', first, last)

    def xview(self, *args):
       apply(self.tk.call, (self._w, 'xview') + args)

    def yview(self, *args):
       apply(self.tk.call, (self._w, 'yview') + args)

class Tree(TixWidget):
    """Tree - The tixTree widget can be used to display hierachical
    data in a tree form. The user can adjust
    the view of the tree by opening or closing parts of the tree."""

    def __init__(self, master=None, cnf={}, **kw):
       TixWidget.__init__(self, master, 'tixTree',
                        ['options'], cnf, kw)
       self.subwidget_list['hlist'] = _dummyHList(self, 'hlist')
       self.subwidget_list['vsb'] = _dummyScrollbar(self, 'vsb')
       self.subwidget_list['hsb'] = _dummyScrollbar(self, 'hsb')

    def autosetmode(self):
       self.tk.call(self._w, 'autosetmode')

    def close(self, entrypath):
       self.tk.call(self._w, 'close', entrypath)

    def getmode(self, entrypath):
       return self.tk.call(self._w, 'getmode', entrypath)

    def open(self, entrypath):
       self.tk.call(self._w, 'open', entrypath)

    def setmode(self, entrypath, mode='none'):
       self.tk.call(self._w, 'setmode', entrypath, mode)


# Could try subclassing Tree for CheckList - would need another arg to init
class CheckList(TixWidget):
    """The CheckList widget
    displays a list of items to be selected by the user. CheckList acts
    similarly to the Tk checkbutton or radiobutton widgets, except it is
    capable of handling many more items than checkbuttons or radiobuttons.
    """

    def __init__(self, master=None, cnf={}, **kw):
        TixWidget.__init__(self, master, 'tixCheckList',
                           ['options'], cnf, kw)
        self.subwidget_list['hlist'] = _dummyHList(self, 'hlist')
        self.subwidget_list['vsb'] = _dummyScrollbar(self, 'vsb')
        self.subwidget_list['hsb'] = _dummyScrollbar(self, 'hsb')
        
    def autosetmode(self):
        self.tk.call(self._w, 'autosetmode')

    def close(self, entrypath):
        self.tk.call(self._w, 'close', entrypath)

    def getmode(self, entrypath):
        return self.tk.call(self._w, 'getmode', entrypath)

    def open(self, entrypath):
        self.tk.call(self._w, 'open', entrypath)

    def getselection(self, mode='on'):
        '''Mode can be on, off, default'''
        self.tk.call(self._w, 'getselection', mode)

    def getstatus(self, entrypath):
        self.tk.call(self._w, 'getstatus', entrypath)

    def setstatus(self, entrypath, mode='on'):
        self.tk.call(self._w, 'setstatus', entrypath, mode)


###########################################################################
### The subclassing below is used to instantiate the subwidgets in each ###
### mega widget. This allows us to access their methods directly.       ###
###########################################################################

class _dummyButton(Button, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)

class _dummyCheckbutton(Checkbutton, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)

class _dummyEntry(Entry, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)

class _dummyFrame(Frame, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)

class _dummyLabel(Label, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)

class _dummyListbox(Listbox, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)

class _dummyMenu(Menu, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)

class _dummyMenubutton(Menubutton, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)

class _dummyScrollbar(Scrollbar, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)

class _dummyText(Text, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)

class _dummyScrolledListBox(ScrolledListBox, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)
       self.subwidget_list['listbox'] = _dummyListbox(self, 'listbox')
       self.subwidget_list['vsb'] = _dummyScrollbar(self, 'vsb')
       self.subwidget_list['hsb'] = _dummyScrollbar(self, 'hsb')

class _dummyHList(HList, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)

class _dummyScrolledHList(ScrolledHList, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)
       self.subwidget_list['hlist'] = _dummyHList(self, 'hlist')
       self.subwidget_list['vsb'] = _dummyScrollbar(self, 'vsb')
       self.subwidget_list['hsb'] = _dummyScrollbar(self, 'hsb')

class _dummyTList(TList, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)

class _dummyComboBox(ComboBox, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)
       self.subwidget_list['entry'] = _dummyEntry(self, 'entry')
       self.subwidget_list['arrow'] = _dummyButton(self, 'arrow')
       # I'm not sure about this destroy_physically=0 in all cases;
       # it may depend on if -dropdown is true; I've added as a trial
       self.subwidget_list['slistbox'] = _dummyScrolledListBox(self,
                                                        'slistbox',
                                                        destroy_physically=0)
       self.subwidget_list['listbox'] = _dummyListbox(self, 'listbox',
                                                 destroy_physically=0)

class _dummyDirList(DirList, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)
       self.subwidget_list['hlist'] = _dummyHList(self, 'hlist')
       self.subwidget_list['vsb'] = _dummyScrollbar(self, 'vsb')
       self.subwidget_list['hsb'] = _dummyScrollbar(self, 'hsb')

class _dummyDirSelectBox(DirSelectBox, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)
       self.subwidget_list['dirlist'] = _dummyDirList(self, 'dirlist')
       self.subwidget_list['dircbx'] = _dummyFileComboBox(self, 'dircbx')

class _dummyExFileSelectBox(ExFileSelectBox, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)
       self.subwidget_list['cancel'] = _dummyButton(self, 'cancel')
       self.subwidget_list['ok'] = _dummyButton(self, 'ok')
       self.subwidget_list['hidden'] = _dummyCheckbutton(self, 'hidden')
       self.subwidget_list['types'] = _dummyComboBox(self, 'types')
       self.subwidget_list['dir'] = _dummyComboBox(self, 'dir')
       self.subwidget_list['dirlist'] = _dummyScrolledListBox(self, 'dirlist')
       self.subwidget_list['file'] = _dummyComboBox(self, 'file')
       self.subwidget_list['filelist'] = _dummyScrolledListBox(self, 'filelist')

class _dummyFileSelectBox(FileSelectBox, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)
       self.subwidget_list['dirlist'] = _dummyScrolledListBox(self, 'dirlist')
       self.subwidget_list['filelist'] = _dummyScrolledListBox(self, 'filelist')
       self.subwidget_list['filter'] = _dummyComboBox(self, 'filter')
       self.subwidget_list['selection'] = _dummyComboBox(self, 'selection')

class _dummyFileComboBox(ComboBox, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)
       self.subwidget_list['dircbx'] = _dummyComboBox(self, 'dircbx')

class _dummyStdButtonBox(StdButtonBox, TixSubWidget):
    def __init__(self, master, name, destroy_physically=1):
       TixSubWidget.__init__(self, master, name, destroy_physically)
       self.subwidget_list['ok'] = _dummyButton(self, 'ok')
       self.subwidget_list['apply'] = _dummyButton(self, 'apply')
       self.subwidget_list['cancel'] = _dummyButton(self, 'cancel')
       self.subwidget_list['help'] = _dummyButton(self, 'help')

class _dummyNoteBookFrame(NoteBookFrame, TixSubWidget):
    def __init__(self, master, name, destroy_physically=0):
       TixSubWidget.__init__(self, master, name, destroy_physically)

########################
### Utility Routines ###
########################

# Returns the qualified path name for the widget. Normally used to set
# default options for subwidgets. See tixwidgets.py
def OptionName(widget):
    return widget.tk.call('tixOptionName', widget._w)

# Called with a dictionary argument of the form
# {'*.c':'C source files', '*.txt':'Text Files', '*':'All files'}
# returns a string which can be used to configure the fsbox file types
# in an ExFileSelectBox. i.e.,
# '{{*} {* - All files}} {{*.c} {*.c - C source files}} {{*.txt} {*.txt - Text Files}}'
def FileTypeList(dict):
    s = ''
    for type in dict.keys():
       s = s + '{{' + type + '} {' + type + ' - ' + dict[type] + '}} '
    return s

# Still to be done:
class CObjView(TixWidget):
    """This file implements the Canvas Object View widget. This is a base
    class of IconView. It implements automatic placement/adjustment of the
    scrollbars according to the canvas objects inside the canvas subwidget.
    The scrollbars are adjusted so that the canvas is just large enough
    to see all the objects.
    """
    pass

