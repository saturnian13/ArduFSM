"""Module for reading and generating TrialSpeak

For instance, this module contains functions for reading log files, splitting
them by trial, and also for generating commands to send to the arduino.
"""
import pandas, numpy as np, my
import StringIO

ack_token = 'ACK'
release_trial_token = 'RELEASE_TRL'
trial_released_token = 'TRL_RELEASED'
start_trial_token = 'TRL_START'
trial_param_token = 'TRLP'
trial_result_token = 'TRLR'

# dictionary for actions
# this must match the arduino code
LEFT = 1
RIGHT = 2
NOGO = 3

# dictionary for outcomes
# this assumes two-alternative choice
HIT = 1
ERROR = 2
SPOIL = 3

# for boolean parameters
YES = 3
NO = 2
MD = 0 # "must-define"

## Reading functions
def load_splines_from_file(filename):
    """Reads lines from file and split into list of lists by trial"""
    # Read lines
    lines = read_lines_from_file(filename)
    
    # Split by trial
    splines = split_by_trial(lines)

    return splines

def split_by_trial(lines):
    """Splits lines from logfile into list of lists by trial.

    Returns: splines, a list of list of lines, each beginning with
    TRIAL START (which is when the current trial params are determined).
    
    Note that this means that the first entry (if it exists) will always be 
    setup info, not trial info.
    """
    if len(lines) == 0:
        return [[]]
    
    # Find the trial start lines
    # This could be done in a couple of ways. Which is most efficient?
    # This method: split by space, check for token in position 1
    trial_starts = [0]
    for nline, line in enumerate(lines):        
        sp_line = line.split()
        if len(sp_line) > 1 and sp_line[1] == start_trial_token:
            trial_starts.append(nline)

    # Now iterate over trial_starts and append the chunks
    splines = []
    for nstart in range(len(trial_starts)-1):
        splines.append(lines[trial_starts[nstart]:trial_starts[nstart+1]])
    
    # Append the leftovers as the last trial, if there is anything
    if len(trial_starts) >= 1:
        splines.append(lines[trial_starts[-1]:])
    
    return splines
    
def read_lines_from_file(filename):
    """Reads all lines from file and returns as list"""
    with file(filename) as fi:
        lines = fi.readlines()
    return lines


## Parsing functions
def parse_lines_into_df(lines):
    """Parse every line into time, command, and argument.
    
    Consider replacing this with read_logfile_into_df
    
    In trial speak, each line has the same format: the time in milliseconds,
    space, a string command, space, an optional argument. This function parses
    each line into those three components and returns as a dataframe.
    """
    # Split each line
    rec_l = []
    for line in lines:
        sp_line = line.split()
        
        # Skip anything with no strings or without a time argument first
        try:
            int(sp_line[0])
        except (IndexError, ValueError):
            continue
        
        # If longer than 3, join the 3rd to the end into a single argument
        if len(sp_line) > 3:
            sp_line = [sp_line[0], sp_line[1], ' '.join(sp_line[2:])]
        rec_l.append(sp_line)
    
    # DataFrame it and convert string times to integer
    if len(rec_l) == 0:
        raise ValueError("cannot extract any lines")
    df = pandas.DataFrame(rec_l, columns=['time', 'command', 'argument'])
    df['time'] = df['time'].astype(np.int)
    return df

def parse_lines_into_df_split_by_trial(lines, verbose=False):
    """Like parse_lines_into_df but split by trial
    
    We drop everything before the first TRL_START token.
    If there is no TRL_START token, return None.
    
    This can be slow if each trial has to be processed separately.
    Consider replacing this with read_logfile_into_df
    """
    # Parse
    df = parse_lines_into_df(lines)
    
    # Debug
    n_lost_lines = len(lines) - len(df)
    if verbose and n_lost_lines != 0:
        print "warning: lost %d lines" % n_lost_lines
    
    # Split by trial
    trl_start_idxs = my.pick_rows(df, command=start_trial_token).index
    
    # Return [empty df] if nothing
    if len(trl_start_idxs) == 0:
        return None
    
    # Split df
    # In each case, we include the first line but exclude the last
    res = []
    for nidx in range(len(trl_start_idxs) - 1):
        # Slice out, including first but excluding last
        slc = df.ix[trl_start_idxs[nidx]:trl_start_idxs[nidx + 1] - 1]
        res.append(slc)
    
    # Append anything left at the end (current trial, generally)
    res.append(df.ix[trl_start_idxs[-1]:])

    return res

def my_replace(ser, d, nanval='nanval'):
    """My version of pandas.Series.replace
    
    There is some bug in pandas 0.12.0 replace function. This is a
    workaround.
    
    ser : Series
    d   : dict. 
    
    First ser is converted to an object data-type, because this is only
    intended to translate numeric data into strings.
    
    Then, for each key, value in d, all instances of key in ser are replaced
    with value.
    
    Finally, because we can't directly compare with np.nan, all null rows
    of ser are replaced with nanval.    
    """
    # Need to convert to object incase new val has different type
    ser = ser.copy().astype(np.object)
    
    # Replace each key, value
    for val, new_val in d.items():
        ser[ser == val] = new_val
        if pandas.isnull(val):
            raise ValueError("cannot compare to nan")
    
    # Replace nans
    ser[ser.isnull()] = nanval
    
    return ser


def translate_trial_matrix(trial_matrix):
    """Replace shorthand with longhand, eg, resp -> response."""
    trial_matrix = trial_matrix.copy()
    trial_matrix = trial_matrix.rename(columns={
        'rwsd': 'rewside',
        'resp': 'choice',
        'outc': 'outcome',
        'srvpos': 'servo_pos',
        'stppos': 'stepper_pos'
        })
    
    
    # How to deal with current trial here?
    if 'outcome' in trial_matrix:
        trial_matrix['outcome'] = my_replace(trial_matrix['outcome'], {
            HIT: 'hit', ERROR: 'error', SPOIL: 'spoil'},
            nanval='curr')
    if 'choice' in trial_matrix:
        trial_matrix['choice'] = my_replace(trial_matrix['choice'], {
            LEFT: 'left', RIGHT: 'right', NOGO: 'nogo'},
            nanval='curr')
    if 'rewside' in trial_matrix:
        trial_matrix['rewside'] = my_replace(trial_matrix['rewside'], {
            LEFT: 'left', RIGHT: 'right', NOGO: 'nogo'})
    if 'isrnd' in trial_matrix:
        assert trial_matrix['isrnd'].isin([YES, NO]).all()
        trial_matrix['isrnd'] = (trial_matrix['isrnd'] == YES)

    return trial_matrix
    

def get_trial_start_time(parsed_lines):
    """Returns the time of the start of the trial in seconds"""
    rows = my.pick_rows(parsed_lines, command=start_trial_token)
    
    if len(rows) > 1:
        raise ValueError("too many trial start lines")
    elif len(rows) == 0:
        return None
    else:
        return int(rows['time'].iat[0]) / 1000.
    
def get_trial_release_time(parsed_lines):
    """Returns the time of trial release in seconds"""
    rows = my.pick_rows(parsed_lines, command=trial_released_token)
    if len(rows) > 1:
        raise ValueError("too many trial relased lines")
    elif len(rows) == 0:
        return None
    else:
        return int(rows['time'].iat[0]) / 1000.

def get_trial_parameters(parsed_lines, command_string=trial_param_token):
    """Returns the value of trial parameters"""
    rec = {}
    rows = my.pick_rows(parsed_lines, command=command_string)
    for arg in rows['argument'].values:
        name, value = arg.split()
        rec[name.lower()] = int(value)
    return rec

def get_trial_results(parsed_lines, command_string=trial_result_token):
    """Returns the value of trial results"""
    # We can use the same code but another token
    return get_trial_parameters(parsed_lines, command_string=command_string)

def check_if_trial_released(trial):
    """Checks if ACK RELEASE_TRL is in trial"""
    for line in trial:
        sp_line = line.split()
        if len(sp_line) == 3 and sp_line[1:3] == [ack_token, release_trial_token]:
            return True
    return False


def has_lick(s):
    return 'EVENT TOUCHED 1' in s or 'EVENT TOUCHED 2' in s

def has_lick_num(s, num):
    return 'EVENT TOUCHED %d' % num in s

def get_lick_times(spline, num):
    res = []
    masked_splines = [line for line in spline if has_lick_num(line, num)]
    for line in masked_splines:
        res.append(int(line.split()[0]) / 1000.)
    return np.array(res)

def identify_state_change_times(behavior_filename=None, logfile_df=None,
    state0=None, state1=None,
    error_on_multiple_changes=False, warn_on_multiple_changes=True, 
    command='ST_CHG2'):
    """Return time that state changed from state0 to state1 on each trial
    
    behavior_filename : name of logfile.
        Only used if logfile_df is not None
    logfile_df : result of read_logfile_into_df
    state0 : state before change
        If None, can be any
    state1 : state after change
        If None, can be any
    
    Uses read_logfile_into_df to read the file
    and get_commands_from_parsed_lines to parse the lines
    and then parses the states in the resulting dataframe.
    
    ST_CHG2 is used, because this is the time of the end of the last
    call of the state before the change. ST_CHG gives you the time of the
    start of that call.
    
    If multiple times are found for a trial, only the first is returned.
    If no times are found for a trial, there will be no entry for that trial
    in the returned data.
    
    Returns: pandas Series indexed by trial with the state change time
        for each trial. The values will be a number of milliseconds
        as an integer.
    """
    # Get logfile_df
    if logfile_df is None:
        logfile_df = read_logfile_into_df(behavior_filename)
    
    # Get the state change times
    state_change_cmds = get_commands_from_parsed_lines(
        logfile_df, command)
    
    # Drop any from trial "-1"
    state_change_cmds = state_change_cmds[state_change_cmds.trial != -1]
    
    # Get the ones corresponding to servo retract
    state_change_cmds = my.pick_rows(state_change_cmds, 
        arg0=state0, arg1=state1)
    
    # Group by trial
    gobj = state_change_cmds.groupby('trial')

    # Error check
    if (gobj.apply(len) != 1).any():
        if error_on_multiple_changes:
            raise ValueError("non-unique state change on some trials")
        if warn_on_multiple_changes:
            print "warning: non-unique state change on some trials"
    
    # Take the first from each trial
    time_by_trial = gobj.first()
    
    return time_by_trial['time']

def identify_state_change_times_new(*args, **kwargs):
    print ("warning: identify_state_change_times_new is deprecated, "
        "use identify_state_change_times")
    return identify_state_change_times(*args, **kwargs)
    
def identify_servo_retract_times(behavior_filename):
    """Identify transition to 13 or 14.
    
    On error trials we get one of each, so take the first one
    """
    return identify_state_change_times(behavior_filename, 
        state0=None, state1=[13, 14], 
        error_on_multi=False)

## Writing functions
def command_set_parameter(param_name, param_value):
    """Returns the command to use to set a parameter.
    
    TODO: error checking here for param_value as int, param_name as string
    without spaces, etc.
    """
    if int(param_value) == 0:
        raise ValueError("cannot send zero")
    return 'SET %s %s' % (param_name, str(int(param_value)))

def command_release_trial():
    """Returns the command to use to release the current trial."""
    return release_trial_token




## This is a reimplementation of the make_trial_matrix function
## Careful if the logfile is corrupted, I think the parsing will lead
## to a segfault here.
## Ideally we could parse the entire logfile into something with a variable
## number of columns?
def get_trial_parameters2(pldf, logfile_lines):
    """Parse out TRLP lines using pldf and logfile_lines.
    
    Returns df pivoted on trial.
    """
    # choose trlp lines
    trlp_idxs = my.pick(pldf, command='TRLP')
    if len(trlp_idxs) == 0:
        return None

    # read into table
    trlp_strings_a = np.asarray(logfile_lines)[trlp_idxs]
    sio = StringIO.StringIO("".join(trlp_strings_a))
    trlp_df = pandas.read_table(sio, sep=' ', 
        names=('time', 'command', 'trlp_name', 'trlp_value'),
        )

    # Add trial marker and pivot on trial.
    # Should we check for dups / missings?
    trlp_df['trial'] = pldf['trial'][trlp_idxs].values
    trlps_by_trial = trlp_df.pivot_table(
        index='trial', values='trlp_value', columns='trlp_name')
    
    return trlps_by_trial

def get_trial_results2(pldf, logfile_lines):
    """Parse out TRLR lines using pldf and logfile_lines.
    
    Returns df pivoted on trial.
    """
    # choose trlr lines
    trlr_idxs = my.pick(pldf, command='TRLR')
    if len(trlr_idxs) == 0:
        return None

    # read into table
    trlr_strings_a = np.asarray(logfile_lines)[trlr_idxs]
    sio = StringIO.StringIO("".join(trlr_strings_a))
    trlr_df = pandas.read_table(sio, sep=' ', 
        names=('time', 'command', 'trlr_name', 'trlr_value'),
        )

    # Add trial marker and pivot on trial.
    # Should we check for dups / missings?
    trlr_df['trial'] = pldf['trial'][trlr_idxs].values
    trlrs_by_trial = trlr_df.pivot_table(
        index='trial', values='trlr_value', columns='trlr_name')
    return trlrs_by_trial

def get_trial_timings(pldf, logfile_lines, 
    token_l=(start_trial_token, trial_released_token),
    ):
    """Parse out lines with trial timing tokens using pldf and logfile_lines.
    
    Returns df pivoted on trial.
    """
        
    # choose lines
    idxs = pldf.index[pldf['command'].isin(token_l)]
    if len(idxs) == 0:
        return None
    
    # read into table
    sio = StringIO.StringIO("".join([logfile_lines[idx] for idx in idxs]))
    parsed_command_lines = pandas.read_table(sio, sep=' ',
        names=('time', 'command'))
    
    # Add trial marker
    parsed_command_lines['trial'] = pldf['trial'][idxs].values
    
    # Pivot by trial
    piv = parsed_command_lines.pivot_table(index='trial', values='time', 
        columns='command') / 1000.
    
    # Drop the "-1" trial
    piv = piv.drop(-1)
    
    return piv

def make_trials_matrix_from_logfile_lines2(logfile_lines,
    always_insert=('resp', 'outc')):
    """Parse out the parameters and outcomes from the lines in the logfile
    
    This was written to be a more optimized version of 
    make_trials_matrix_from_logfile_lines and is used in trial_setter.
    Should combine this with TrialMatrix.make_trial_matrix_from_logfile_lines
    and just make one thing that does this function.
    
    For each trial, the following parameters are extracted:
        trial_start : time in seconds at which TRL_START was issued
        trial_released: time in seconds at which trial was released
        All parameters and results listed for each trial.
    
    If the first trial has not started yet, an empty DataFrame is returned.

    The columns in always_insert are always inserted, even if they 
    weren't present. They will be inserted with np.nan, so the dtype 
    should be numerical, not stringy. The main use-case is the response 
    columns which are missing during the first trial but which most 
    code assumes exists.
    
    TODO: bug here where malformed lines cause pldf and lines not to match
    up anymore.
    """
    if len(logfile_lines) == 0:
        return pandas.DataFrame(np.zeros((0, len(always_insert))),
            columns=always_insert)
    
    # Parse
    pldf = parse_lines_into_df(logfile_lines)
    if len(pldf) == 0:
        return pandas.DataFrame(np.zeros((0, len(always_insert))),
            columns=always_insert)

    # Find the boundaries between trials in logfile_lines
    trl_start_idxs = my.pick_rows(pldf, 
        command=start_trial_token).index
    if len(trl_start_idxs) == 0:
        return pandas.DataFrame(np.zeros((0, len(always_insert))),
            columns=always_insert)
    
    # Assign trial numbers. The first chunk of lines are pre-session setup,
    # so subtract 1 to make that trial "-1".
    # Use side = 'right' to place TRL_START itself correctly
    pldf['trial'] = np.searchsorted(np.asarray(trl_start_idxs), 
        np.asarray(pldf.index), side='right') - 1    
    
    # Extract various things
    trlps_by_trial = get_trial_parameters2(pldf, logfile_lines)
    trlrs_by_trial = get_trial_results2(pldf, logfile_lines)
    tt_by_trial = get_trial_timings(pldf, logfile_lines)
    
    # Join
    res = pandas.concat(
        [trlps_by_trial, trlrs_by_trial, tt_by_trial], axis=1,
        verify_integrity=True)
    
    # Lower case the names
    res.columns = [col.lower() for col in res.columns]
    
    # rename timings names
    res = res.rename(columns={
        'trl_start': 'start_time',
        'trl_released': 'release_time',
        })

    # Define duration
    if 'release_time' in res.columns and 'start_time' in res.columns:
        res['duration'] = res['release_time'] - res['start_time']
    else:
        res['release_time'] = pandas.Series([], dtype=np.float)
        res['start_time'] = pandas.Series([], dtype=np.float)
        res['duration'] = pandas.Series([], dtype=np.float)
    
    # Reorder
    ordered_cols = ['start_time', 'release_time', 'duration']
    for col in sorted(res.columns):
        if col not in ordered_cols:
            ordered_cols.append(col)
    res = res[ordered_cols]
    
    # Avoid SettingWithCopyWarning below
    res = res.copy()
    
    # Insert always_insert
    for col in always_insert:
        if col not in res:
            res[col] = np.nan
    
    # Name index
    res.index.name = 'trial'
    
    return res


def read_logfile_into_df(logfile, nargs=4, add_trial_column=True,
    unsorted_times_action='warn'):
    """Read logfile into a DataFrame
    
    Something like this should probably be the preferred way to read the 
    lines into a structured data frame.
    
    Use get_commands_from_parsed_lines to parse the arguments, eg,
    converting to numbers.
    
    Each line in the file will be a row in the data frame.
    Each line is separated by whitespace into the different columns.
    Thus, the first column will be "time", the second "command", and the
    rest "arguments".
    
    nargs : how many argument columns to add. Lines that contain more arguments
        than this will be silently truncated! Lines with fewer will be padded
        with None.
    add_trial_column : optionally add a column for the trial number of 
        each line. Lines before the first trial begins have trial number -1.
    
    unsorted_times_action : 'ignore', 'warn', 'error'
        The times "should" be sorted but frequently aren't.
        This is always the case for some commands, and less frequently
        the case when the first digit seems to have been dropped.
    
    The dtype will always be int for the time column and object (ie, string)
    for every other column. This is to ensure consistency. You may want
    to coerce certain columns into numeric dtypes.
    """
    # Determine how many argument columns to use
    arg_cols = ['arg%d' % n for n in range(nargs)]
    all_cols = ['time', 'command'] + arg_cols
    
    # Set dtypes
    dtype_d = {'time': np.int, 'command': np.object}
    for col in arg_cols:
        dtype_d[col] = np.object
    
    # Read. Important to avoid reading header of index or you can get
    # weird errors here, like unnamed columns.
    rdf = pandas.read_table(logfile, sep=' ', names=all_cols, 
        index_col=False, header=None)
    if not np.all(rdf.columns == all_cols):
        raise IOError("cannot read columns correctly from logfile")
    
    # Convert time to integer and drop malformed lines
    #new_time = rdf['time'].convert_objects(convert_numeric=True)
    new_time = pandas.to_numeric(rdf['time'], errors='coerce')
    if new_time.isnull().any():
        print "warning: malformed time string at line(s) %r" % (
            new_time.index[new_time.isnull()].values)
    rdf['time'] = new_time
    rdf = rdf.ix[~rdf['time'].isnull()]
    rdf['time'] = rdf['time'].astype(np.int)
    
    # Convert dtypes. We have to do it here, because if done during reading
    # it will crash on mal-formed dtypes. Could catch that error and then
    # run this...
    # Well this isn't that useful because it leaves dtypes messed up. Need
    # to find and drop the problematic lines.
    for col, dtyp in dtype_d.items():
        try:
            rdf[col] = rdf[col].astype(dtyp)
        except ValueError:
            raise IOError("cannot coerce %s to %r" % (col, dtyp))
    
    # Join on trial number
    if add_trial_column:
        # Find the boundaries between trials in logfile_lines
        trl_start_idxs = my.pick_rows(rdf, 
            command=start_trial_token).index
        if len(trl_start_idxs) > 0:            
            # Assign trial numbers. The first chunk of lines are 
            # pre-session setup, so subtract 1 to make that trial "-1".
            # Use side = 'right' to place TRL_START itself correctly
            rdf['trial'] = np.searchsorted(np.asarray(trl_start_idxs), 
                np.asarray(rdf.index), side='right') - 1        
    
    # Error check
    # Very commonly the ACK TRL_RELEASED, SENH, AAR_L, and AAR_R commands
    # are out of order. So ignore this for now.
    # Somewhat commonly, there is a missing first digit of the time, for
    # some reason.
    rrdf = rdf[
        ~rdf.command.isin(['DBG', 'ACK', 'SENH']) &
        ~rdf.arg0.isin(['AAR_L', 'AAR_R'])
        ]
    unsorted_times = rrdf['time'].values
    bad_args = np.where(np.diff(unsorted_times) < 0)[0]
    if len(bad_args) > 0:
        first_bad_arg = bad_args[0]
        pre_bad_arg = np.max([first_bad_arg - 2, 0])
        post_bad_arg = np.min([first_bad_arg + 2, len(rrdf)])
        bad_rows = rrdf.ix[rrdf.index[pre_bad_arg]:rrdf.index[post_bad_arg]]
        error_string = "unsorted times in logfile %s, starting at line %d" % (
            logfile, bad_args[0])

        if unsorted_times_action == 'warn':
            print error_string
        elif unsorted_times_action == 'error':
            raise ValueError(error_string)
        elif unsorted_times_action == 'ignore':
            pass
        else:
            raise ValueError("unknown action for unsorted times")
    
    return rdf
    
def get_commands_from_parsed_lines(parsed_lines, command,
    arg2dtype=None):
    """Return only those lines that match "command" and set dtypes.
    
    parsed_lines : result of read_logfile_into_df
    command : 'ST_CHG', 'ST_CHG2', 'TCH', etc.
        This is used to select rows from parsed_lines. For known arguments,
        we can also use this to set arg2dtype.
    arg2dtype : dict explaining which args to keep and what dtype to convert
        e.g., {'arg0': np.int, 'arg1': np.float}
    
    Returns:
        DataFrame with one row for each matching command, and just the
        requested columns. We always include 'time', 'command', and
        'trial' if available
    
    Can use something like this to group the result by trial and arg0:
    tt2licks = lick_times.groupby(['trial', 'arg0']).groups
    for (trial, lick_type) in tt2licks:
        tt2licks[(trial, lick_type)] = \
            ldf.loc[tt2licks[(trial, lick_type)], 'time'].values / 1000.    
    
    See BeWatch.misc for other examples of task-specific logic
    """
    # Pick
    res = my.pick_rows(parsed_lines, command=command)
    
    # Decide which columns to keep and how to coerce
    if command == 'ST_CHG2':
        if arg2dtype is None:
            arg2dtype = {'arg0': np.int, 'arg1': np.int}
    elif command == 'ST_CHG':
        if arg2dtype is None:
            arg2dtype = {'arg0': np.int, 'arg1': np.int}
    elif command == 'TCH':
        arg2dtype = {'arg0': np.int}
    
    if arg2dtype is None:
        raise ValueError("must provide arg2dtype")
    
    # Keep only the columns we want
    keep_cols = ['time', 'command'] + sorted(arg2dtype.keys())
    if 'trial' in res.columns:
        keep_cols.append('trial')    
    res = res[keep_cols]

    # Coerce dtypes
    for argname, dtyp in arg2dtype.items():
        try:
            res[argname] = res[argname].astype(dtyp)
        except ValueError:
            print "warning: cannot coerce column %s to %r" % (argname, dtyp)

    return res