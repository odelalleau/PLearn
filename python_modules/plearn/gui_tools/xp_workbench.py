# xp_workbench.py
# Copyright (C) 2008 by Nicolas Chapados
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   3. The name of the authors may not be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#  NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  This file is part of the PLearn library. For more information on the PLearn
#  library, go to the PLearn Web site at www.plearn.org

# Author: Nicolas Chapados

## System imports
import ctypes
import inspect
import os.path
import sys
import threading
import datetime

_HAS_DISPLAY_ = os.environ.has_key("DISPLAY")

## Traits
from enthought.traits.api          import *
if _HAS_DISPLAY_:
    from enthought.traits.ui.api       import *
    from enthought.traits.ui.menu      import NoButtons
    from enthought.pyface.api          import confirm, YES
    from console_logger import ConsoleLogger
else:
    ## Dummy definitions for some Traits UI stuff
    def Button(*args, **kwargs): pass
    def Group (*args, **kwargs): pass
    def HSplit(*args, **kwargs): pass
    def Item  (*args, **kwargs): pass
    def VSplit(*args, **kwargs): pass
    def View  (*args, **kwargs): pass
    def ListEditor (*args, **kwargs): pass
    def ValueEditor(*args, **kwargs): pass
    NoButtons = None
    confirm   = None
    YES       = None
    Directory = Str

    class ConsoleLogger(object): pass
    class Handler(object): pass
    


## If there is no display, set the matplotlib backend to Agg
if not _HAS_DISPLAY_:
    import matplotlib
    matplotlib.use('Agg')


#####  ExperimentContext  ###################################################

class ExperimentContext(HasTraits):
    """Contains the context of a single experiment -- either a running one
    or a reloaded one.
    """
    ## Complete path to experiment directory
    expdir = Directory

    ## Whether we are using a GUI interface or not
    gui = false

    ## Copy of experiment parameters
    script_params = Instance(HasTraits, ())

    ## Function to call to run experiment
    expfunc = Function

    ## The console logger (default value is None)
    console_logger = Instance(ConsoleLogger)

    ## All tabs
    _all_tabs = List(HasTraits, desc="Set of tabs associated with experiment",
                     editor=ListEditor(use_notebook=True,
                                       deletable=False,
                                       dock_style="tab",
                                       page_name=".title"))

    def run_experiment(self):
        """Run the current expfunc with current script_params."""
        self.expfunc(self.script_params, self)


    def figure(self, title="Figure", **kwargs):
        """Return a new Matplotlib figure and add it to the notebook.

        Note that you must paint on this figure using the Matplotlib OO
        API, not pylab.  Apart from 'title', the other **kwargs arguments
        are passed to matplotlib figure constructor.
        """
        if self.gui:
            ## Late import to allow backend to be chosen
            from mpl_figure_editor import TraitedFigure
            f = TraitedFigure(title=title, **kwargs)
            f.title = title
            f.figure.__traited_figure = f
            return f.figure
        else:
            ## Late import to allow backend to be chosen
            import pylab
            f = pylab.figure(**kwargs)
            f._title = title
            return f


    def show(self, fig):
        """Take a figure returned by Figure() and show it on screen.

        Depending on whether we are under a GUI or not, the behavior
        changes.
        """
        if self.gui:
            self._all_tabs.append(fig.__traited_figure)
        else:
            ## Late import to allow backend to be chosen
            from mpl_utilities import showOrSave

            ## Determine a semi-intelligent name to save the
            ## figure under, if we are indeed saving figures.
            figname = getattr(fig, "_title", "figure").lower().replace(" ", "_")
            figname = os.path.join(self.expdir, figname)

            i = 1
            while True:
                if os.path.exists(figname+("_%d.png"%i)):
                    i += 1
                else:
                    figname += "_%d.png" % i
                    break
                
            showOrSave(filename = figname, use_environment=True)

    
    @property
    def _display_expdir(self):
        """Shortened version of expdir suitable for display."""
        return os.path.basename(self.expdir)

    def _console_logger_changed(self, old, new):
        """Keep the logger in the list of tabs."""
        if old is not None and old in self._all_tabs:
            self._all_tabs.remove(old)
        self._all_tabs.append(new)
    
    
    ## Default view
    traits_view = View(Item('_all_tabs@', show_label=False))


class _ConsoleOutput(HasTraits):
    title    = "Output"
    contents = Str

    traits_view = View(Item("contents~",show_label=False))


class _WorkerThread(threading.Thread):
    """Utility class to perform experiment in a separate thread.
    It notifies the workbench when it's done.

    Note: this thread can (theoretically) be killed from outside, which is
    quite nonstandard in Python.  See
    http://sebulba.wikispaces.com/recipe+thread2 for details of how this is
    done.  However it does not seem to work very well for now...
    """
    def __init__(self, xp_context, wkbench):
        def work():
            ## Call user-defined work function, and before quitting
            ## notify that we are done
            xp_context.run_experiment()
            wkbench.curworker = None

        super(_WorkerThread,self).__init__(target=work)
        self.setDaemon(True)     # Allow quitting Python even if thread still running
        self.xp_context = xp_context

    def _get_my_tid(self):
        """determines this (self's) thread id"""
        if not self.isAlive():
            raise threading.ThreadError("the thread is not active")
        
        # do we have it cached?
        if hasattr(self, "_thread_id"):
            return self._thread_id
        
        # no, look for it in the _active dict
        for tid, tobj in threading._active.items():
            if tobj is self:
                self._thread_id = tid
                return tid
        
        raise AssertionError("could not determine the thread's id")
    
    def raise_exc(self, exctype):
        """raises the given exception type in the context of this thread"""
        _async_raise(self._get_my_tid(), exctype)
    
    def terminate(self):
        """raises SystemExit in the context of the given thread, which should 
        cause the thread to exit silently (unless caught)"""
        self.raise_exc(SystemExit)    


#####  Custom Handlers  #####################################################

if _HAS_DISPLAY_:
    class WorkbenchHandler(Handler):
        """Some custom UI code for the main window.
        """
        def close(self, info, is_ok):
            """If user is trying to close the main window, look at whether an
            experiment is currently running.  If so, pop a dialog to ask
            whether it's really OK to close the thing.
            """
            #workbench = info.ui.context['object']
            workbench = info.object
            if workbench.curworker is not None:
                msg = "An experiment is currently running.\n" \
                      "Do you really want to interrupt it and\n" \
                      "close the workbench window?"
                parent = info.ui.control
                return confirm(parent, msg) == YES
            else:
                return True
else:
    class WorkbenchHandler(object):
        pass

#####  ExperimentWorkbench  #################################################

class ExperimentWorkbench(HasTraits) :
    """Manage the interface of a traits-based experiment.
    """
    ## Data traits
    script_params = Instance(HasTraits, (), desc="Script Parameters")
    experiments   = List(ExperimentContext, desc="Set of experiments",
                         editor=ListEditor(use_notebook=True,
                                           deletable=True,
                                           dock_style="tab",
                                           page_name="._display_expdir"))

    expfunc   = Function(desc="Function to run when experiment is running")
    curworker = Instance(threading.Thread, desc="Worker thread, if any is running")

    ## Whether we are running under a gui
    gui = false

    ## Active traits
    launch = Button("Launch Experiment")
    cancel = Button("Cancel")

    ## The main view
    traits_view  = View(
        HSplit(Group(Group(Item("script_params@", show_label=False, springy=False)),
                     #spring,
                     Group(Item("launch", springy=True, enabled_when="curworker is None"),
                           Item("cancel", springy=True, enabled_when="curworker is not None"),
                           show_labels=False, orientation="horizontal")),
               Group(Item("experiments@", dock="fixed", show_label=False, width=300),
                     springy=True)),
        resizable=True, #scrollable=True,
        height=0.75, width=0.75,
        buttons=NoButtons,
        handler=WorkbenchHandler() )

    ## Experiment management
    def _launch_fired(self):
        """Called when the 'launch' button is clicked."""
        context = self._new_xp_context()
        context.console_logger = ConsoleLogger()
        self.curworker = _WorkerThread(context, self)
        self.curworker.start()

    def _new_xp_context(self):
        """Initialize an experiment context
        """
        expdir = None
        if hasattr(self.script_params, 'expdir'):
            expdir = self.script_params.expdir
        expdir_path = self.expdir_name(self.script_params.expdir_root, expdir)
        context = ExperimentContext(expdir = expdir_path,
                                    gui = self.gui,
                                    script_params = self.script_params,
                                    expfunc = self.expfunc)
        self.experiments.append(context)
        return context

    def _cancel_fired(self):
        if self.curworker is not None:
            self.curworker.terminate()
            self.curworker.join()
            self.curworker = None

    def _curworker_changed(self, old, new):
        ## If curworker had an active console logger, disable it
        if old is not None:
            context = old.xp_context
            logger  = context.console_logger
            if logger is not None:
                logger.desactivate_stdout_err_redirect()

        ## And redirect output to the new logger...
        if new is not None:
            context = new.xp_context
            logger = context.console_logger
            if logger is not None:
                logger.activate_stdouterr_redirect()

    
    def run(self, params, func, gui="__AUTO__"):
        """Bind the command-line arguments to the params, show the gui (if
        requested) and run the experiment by calling 'func' in a separate
        thread.

        Arguments:

        - params: Either a class or instance inheriting from HasTraits
          and specifying the parameters (hierarchically) for the experiment.

        - func: Function to be called to run the experiment.  The function
          is called with two arguments: a filled-out params instance
          containing the experiment parameters, and an ExperimentContext
          instance giving, among other things, an experiment directory and
          facility to create Matplotlib figures to be added to the workbench.

        - gui: Either a boolean, or if '__AUTO__', taken from the
          command-line switch --no-gui or --gui.
        """

        ## Instantiate the params container if a class was passed
        if isinstance(params,type):
            params = params()
        assert isinstance(params, HasTraits), \
               "The 'params' argument must be an instance or a class inheriting from HasTraits."

        ## Determine whether a GUI should be used
        if gui == "__AUTO__":
            if "--no-gui" in sys.argv or not _HAS_DISPLAY_:
                gui = False
            else:
                gui = True
        self.gui = gui

        ## Bind the command-line arguments to the parameters
        self.script_params = self.bind(params, sys.argv)
        self.expfunc = func

        ## Run the thing
        if self.gui:
            self.configure_traits()
        else:
            context = self._new_xp_context()
            context.run_experiment()


    @staticmethod
    def bind(params, argv):
        """Bind any arguments in argv that does not start with a '-' to
        elements in params and has the form 'K=V'.  This assumes that
        'params' (and the classes contained therin) inherits from
        HasStrictTraits so that any assignment to inexistant options raises
        an exception.

        If an argument starts with an '@'-sign, it is intrepreted as a filename
        containing extra arguments to insert in the list of arguments. The file is
        opened, and each line that doesn't start with a '#'-sign is taken as a new 
        argument. These argument are inserted in order, where the @filename directive
        was found, with said @filename directive being removed from the list of 
        arguments. It is possible to use this multiple times: @filename2 @filename2

        """

        # First replace all @filename by their contents
        expanded_argv = []
        for arg in argv:
            if arg.startswith('@'):
                f = open(arg[1:], 'rU')
                expanded_argv.extend(line.strip() for line in f if not line.startswith('#'))
                f.close()
            else:
                expanded_argv.append(arg)

        for arg in expanded_argv:
            if arg.startswith('-'):
                continue
            
            if '=' in arg:
                (k,v) = arg.split('=', 1)
                v = v.replace("'", "\\'")

                ## For compatibility between mixed plnamespace and traits,
                ## if the first argument to a compound option does not
                ## exist, skip it without complaining -- it is destined for
                ## plnamespace, which have been processed before
                if '.' in k:
                    k_parts = k.split('.')
                    try:
                        getattr(params, k_parts[0])
                    except AttributeError:
                        continue
                
                if isinstance(eval("params.%s" % k), str):
                    exec("params.%s = '%s'" % (k,v))
                else:
                    ## Potentially dangerous use of 'eval' with
                    ## command-line arguments; expeditive solution for now,
                    ## but may opt for more restricted/lenient parsing in
                    ## the future, like we had for plargs.
                    exec("params.%s = eval('%s')" % (k,v))

        return params


    @staticmethod
    def expdir_name(expdir_root, expdir=None):
        """Return an experiment directory from a root location and possibly a dir name."""
        if expdir is None or expdir == '':
            expdir = datetime.datetime.now().strftime("expdir_%Y%m%d_%H%M%S")
        return os.path.join(expdir_root, expdir)


    @staticmethod
    def print_all_traits(root, out = sys.stdout, prefix=""):
        """Recursively print out all traits in a key=value format.
        Useful for generating metainfo files for experiments.
        """
        traits = root.get()
        for trait_name in sorted(traits.keys()):
            trait_value = traits[trait_name]
            if trait_name in ["trait_added", "trait_modified"] or trait_name.startswith('_'):
                continue
            elif isinstance(trait_value, HasTraits):
                ExperimentWorkbench.print_all_traits(trait_value, out, prefix+trait_name+".")
            else:
                print >>out, ("%-40s" % (prefix+trait_name)) + " = " + str(trait_value)


#####  Top-Level Options  ###################################################

class SingletonOptions(HasStrictTraits):
    """Subclasses of this class are singletons, designed to provide an
    approximate drop-in traits-based replacement to plnamespace.  If you
    don't want or need the singleton behavior, simply have your options
    classes inherit from HasTraits or HasStrictTraits.

    Note: this class inherits from HasStrictTraits since it is intended to
    mimic plnamespace as closely as possible to ease migration.  It should
    be avoided for writing new code.
    """
    class __metaclass__(HasStrictTraits.__metaclass__):
        __instances = {}
        
        def __call__(cls, *args, **kwargs):
            klass_key = str(cls)
            if klass_key not in SingletonOptions.__instances:
                instance = type.__call__(cls, *args, **kwargs)
                SingletonOptions.__instances[klass_key] = instance
                return instance
            else:
                return SingletonOptions.__instances[klass_key]
                

class TopLevelOptions(HasStrictTraits):
    """Utility class from which one can inherit for top-level options.
    It provides a reasonable default traits_view implementation.
    """
    def trait_view(self, name=None, view_element=None):
        if (name or view_element) != None:
            return super(TopLevelOptions, self).trait_view( name=name,
                                                            view_element=view_element )

        items  = [ Item(t+"@") for t in self.trait_names()
                   if t not in [ "expdir_root", "trait_added", "trait_modified" ] ]
        kwargs = { "show_labels":False, "layout":"tabbed" }
        return View(Group(*items, **kwargs))
    

#####  Utilities  ###########################################################

def _async_raise(tid, exctype):
    """raises the exception, performs cleanup if needed"""
    if not inspect.isclass(exctype):
        raise TypeError("Only types can be raised (not instances)")
    res = ctypes.pythonapi.PyThreadState_SetAsyncExc(tid, ctypes.py_object(exctype))
    if res == 0:
        raise ValueError("invalid thread id")
    elif res != 1:
        # """if it returns a number greater than one, you're in trouble, 
        # and you should call it again with exc=NULL to revert the effect"""
        ctypes.pythonapi.PyThreadState_SetAsyncExc(tid, 0)
        raise SystemError("PyThreadState_SetAsyncExc failed")


#####  Test Case  ###########################################################

if __name__ == "__main__":
    class GlobalOpt(HasStrictTraits):
        expdir_root       = Directory(".", auto_set=True,
                                         desc="where the experiment directory should be created")
        expdir            = Str("",      desc="experiment directory name")
        max_train_size    = Trait( -1 ,  desc="maximum size of training set (in days)")
        nhidden           = Trait(3,     desc="number of hidden units")
        weight_decay      = Trait(1e-8,  desc="weight decay to use for neural-net training")

    class MinorOpt(HasStrictTraits):
        earlystop_fraction  = Trait(  0.0, desc="fraction of training set to use for early stopping")
        earlystop_check     = Trait(    1, desc="check early-stopping criterion every N epochs")
        earlystop_minstage  = Trait(    1, desc="minimum optimization stage after which early-stopping can kick in")

    class AllOpt(HasStrictTraits):
        expdir_root = Delegate("GlobalOpt")
        expdir      = Delegate("GlobalOpt")
        GlobalOpt   = Instance(GlobalOpt, ())
        MinorOpt    = Instance(MinorOpt,  ())

        traits_view = View(
            Group(Item("GlobalOpt@"),
                  Item("MinorOpt@" ),
                  show_labels=False, layout="tabbed"))

    def f(params, context):
        print "Now running a very complex experiment"
        print "params.GlobalOpt.nhidden = ", params.GlobalOpt.nhidden
        print "context.expdir           = ", context.expdir

        from numpy import sin, cos, linspace, pi
        print "Drawing a figure..."
        f = context.figure()
        axes = f.add_subplot(111)
        t = linspace(0, 2*pi, 200)
        axes.plot(sin(t)*(1+0.5*cos(11*t)), cos(t)*(1+0.5*cos(11*t)))
        context.show(f)

        print "Sleeping during a long computation..."
        sys.stdout.flush()
        try:
            import time
            time.sleep(10)
        finally:
            print "Done."

    ExperimentWorkbench().run(AllOpt, f)

# vim: filetype=python:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
