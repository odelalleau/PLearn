import time, random, os

def writeResults(argdict, costdict, results_amat, need_lock = True):
    """ Write the results of an experiment in a amat file,
        managing the fact that several scripts can try yo write
        at the same time in the amat.
        - argdict: list of hyperparameter names and values.
                   Must be a dictionary such as {'n_hidden':50, 'learning_rate':0.1}
                   OR a list of lists, as [['n_hidden', 'learning_rate'],[50, 0.1]]
                   (the interest of this last format is to keep a given order, while
                    alphabetical order is taken with dicts).
        - costdict: list of costs names and values (same format as argdict)
        - results_amat: names of the amat (string)
                        (with or without the extension 'amat')
    """
    argnames, argvals = getSortedValues(argdict)
    cost_names, cost_vals = getSortedValues(costdict)
    if not results_amat.endswith('.amat'):
        results_amat += '.amat'

    # Create .amat if it does not exist.
    if not os.path.exists(results_amat):
        if need_lock:
            lockFile(results_amat)
        f = open(results_amat, "w")
        f.write('TO_CREATE')
        f.close()
        if need_lock:
            unlockFile(results_amat)

    # Fill data in the .amat file.
    if need_lock:
        lockFile(results_amat)
    f = open(results_amat, 'r+')
    if f.readline() == 'TO_CREATE':
        # Need to write the header.
        f.seek(0)
        f.write('# Fieldnames:\n')
        f.write('#: %s %s\n' % \
                (' '.join(argnames), ' '.join(cost_names)))
    f.seek(0, 2)   # End of file.
    f.write('%s\n' % ' '.join( str(x) for x in argvals + cost_vals ) )
    f.close()
    if need_lock:
        unlockFile(results_amat)

def getSortedValues(names_and_vals):
    if isinstance(names_and_vals,dict):
        keys = names_and_vals.keys()
        keys.sort()
        return keys, [names_and_vals[key] for key in keys]
    elif isinstance(names_and_vals,list):
        assert len(names_and_vals) == 2
        assert type(names_and_vals[0]) in [list,str]
        if isinstance(names_and_vals[0],str):
            names_and_vals[0] = names_and_vals[0].split()
        assert isinstance(names_and_vals[1],list)
        assert len(names_and_vals[0]) == len(names_and_vals[1])
        return names_and_vals[0], names_and_vals[1]
    else:
        raise TypeError, "argument of getSortedValues (type %s) must be a dict" % type(names_and_vals)+\
                         " or a list of length 2 (of lists of the same length)."

def lockFile(file, timeout = 30, min_wait = 1, max_wait = 5, verbosity = 0):
    """Obtain lock access to the given file. If access is refused by the same
    lock owner during more than 'timeout' seconds, then overrides the current
    lock. If timeout is None, then no timeout is performed.
    The lock is performed by created a 'file.lock' file that contains a unique
    id identifying the owner of the lock.
    When there is already a lock, the process sleeps for a random amount of
    time between min_wait and max_wait seconds before trying again.
    If 'verbosity' is set to 1, then a message will be displayed when we need
    to wait for the lock. If it is set to a value >1, then this message will
    be displayed each time we re-check for the presence of the lock."""
    lock_file = file + '.lock'
    random.seed()
    unique_id = '%s_%s' % (os.getpid(),
                             ''.join( [ str(random.randint(0,9)) \
                                        for i in range(10) ]))
    no_display = verbosity == 0
    while True:
        last_owner = 'no_owner'
        time_start = time.time()
        while os.path.isfile(lock_file):
            try:
                read_owner = open(lock_file).readlines()[0].strip()
            except:
                read_owner = 'failure'
            if last_owner == read_owner:
                if timeout is not None and time.time() - time_start >= timeout:
                    # Timeout exceeded.
                    break
            else:
                last_owner = read_owner
                time_start = time.time()
            if not no_display:
                print 'Waiting for existing lock (by %s)' % read_owner
                if verbosity <= 1:
                    no_display = True
            time.sleep(random.uniform(min_wait, max_wait))

        # Write own id into lock file.
        lock_write = open(lock_file, 'w')
        lock_write.write(unique_id + '\n')
        lock_write.close()
        time.sleep(1) # Safety wait.
        # Verify noone else tried to claim the lock at the same time.
        owner = open(lock_file).readlines()[0].strip()
        if owner != unique_id:
            # Too bad, try again.
            continue
        else:
            # We got the lock!
            return


def unlockFile(file):
    """Remove current lock on file."""
    lock_file = file + '.lock'
    if os.path.exists(lock_file):
        os.remove(lock_file)
