module Instrumentation;

export {
	const collection_log = "/tmp/bro-profile-collection" &redef;
	const collection_freq = 0 &redef;
	const collection_timeout = 0.0 &redef;
}

event bro_init() {
	if(collection_freq > 0 || collection_timeout > 0) {
		SetCollectionTarget(collection_log);
		if(collection_freq > 0) {
			SetCollectionCount(collection_freq);
		}
		if(collection_timeout > 0) {
			SetCollectionTimer(collection_timeout);
		}
	}
}

event bro_done() {
	if(collection_freq > 0 || collection_timeout > 0) {
		CollectionFlush();
		CollectionFinalize();
	}
}
