module Instrumentation;

export {
	## Log file for chain profile data
	const chain_profile_log = "/tmp/bro-profile-chains" &redef;
	## Whether or not we'll be writing chain data to a file (FIXME: gathering still happens regardless of flag setting ...)
	const chain_profile_enable = F &redef;
}

## If we have enabled chain profiling, open our log file
event bro_init() {
	if(chain_profile_enable) {
		SetChainDataTarget(chain_profile_log);
	}
}

## Write chain information *once* when we exit the application
event bro_done() {
	if(chain_profile_enable) {
		ChainDataWrite();  # This automatically flushes to disk since the write is going to be expensive ...
	}
}
