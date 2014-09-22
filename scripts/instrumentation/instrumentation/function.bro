module Instrumentation;

export {
	## Log into which we'll be writing function timing information.
	## FIXME: either reduce the overhead of recording *or* adjust for the time it takes to do the recording
	const function_profile_log = "/tmp/bro-profile-function" &redef;
	## Whether or not function profiling is currently enabled.
	const function_profile_enable = F &redef;
}

## If function profiling is enabled, open the log file and prepare to write data
event bro_init() {
	if(function_profile_enable) {
		SetFunctionDataTarget(function_profile_log);
	}
}

## Write function profiling data *once* when bro shuts down
event bro_done() {
	if(function_profile_enable) {
		FunctionDataWrite();
		FunctionDataFinalize();
	}
}
