module Instrumentation;

export {
	## Log for periodically collected statistics
	const collection_log = "/tmp/bro-profile-collection" &redef;
	## Write a new entry in the log for each collection_freq packets we observe
	const collection_freq = 0 &redef;
	## Write a new entry in the log for each collection_timeout seconds that elapse (NOTE: network time, not real time)
	const collection_timeout = 0.0 &redef;
}

## If we have either a nonzero collection frequency or a nonzero timeout, open the log file and enable logging
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

## Flush any unwritten log entries and finalize the log (certain formats require a footer to properly close the file)
event bro_done() {
	if(collection_freq > 0 || collection_timeout > 0) {
		CollectionFlush();
		CollectionFinalize();
	}
}
