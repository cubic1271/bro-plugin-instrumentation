module Instrumentation;

export {
	const chain_profile_log = "/tmp/bro-profile-chains" &redef;
	const chain_profile_enable = F &redef;
}

event bro_init() {
	if(chain_profile_enable) {
		SetChainDataTarget(chain_profile_log);
	}
}

event bro_done() {
	if(chain_profile_enable) {
		ChainDataWrite();  # This automatically flushes to disk since the write is going to be expensive ...
	}
}
