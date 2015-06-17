@load instrumentation/instrumentation/collection.bro
@load instrumentation/instrumentation/function.bro
@load instrumentation/instrumentation/chains.bro

redef Instrumentation::collection_freq = 10000;
redef Instrumentation::function_profile_enable = T;
redef Instrumentation::chain_profile_enable = T;

