# Start non-blocking PCR to receive computation task (subregion)
MPI_Pcr_Start()
while true
	# Check non-blocking if receive operation of computation task is complete
	if MPI_Test() is true
		# Start non-blocking PCR receive for possible new computation task
		MPI_Pcr_Start()
		for every column of subregion
			# Compute current row
			compute(row)
			# Check non-blocking if receive operation of new computation task is complete
			if MPI_Test() is true
				# Abort running computation and start with new one
				goto while loop
		# Send results using blocking send operation
		MPI_Send(result)
	else
	# Sleep for 1 ms to reduce CPU workload
	thread.sleep(1)