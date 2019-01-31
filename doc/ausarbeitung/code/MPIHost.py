while true
	# Check for new computation tasks (subregions) in shared datastructure
	if websocket_request_to_mpi not empty
		# Send subregions to worker using non-blocking PCR
		MPI_Pcr_start(subregions)
	# Check non-blocking for incoming computation results from worker
	if MPI_Iprobe() is true
		# Receive computation result using blocking receive operation
		MPI_Recv(result)
		# Store result in shared datastructure
		mpi_to_websocket_result.add(result)