#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2_list-1.png ... cost per operation vs threads and iterations
#	lab2_list-2.png ... threads and iterations that run (un-protected) w/o failure
#	lab2_list-3.png ... threads and iterations that run (protected) w/o failure
#	lab2_list-4.png ... cost per operation vs number of threads
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

# plot of total number of operations per second for each synchronization method (m and s)
set title "List2b-1: Throughput vs. Number of Threads (at 1000 Iterations)"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Throughput (operations/second)"
set logscale y 10
set output 'lab2b_1.png'
set key right top

# grep out only needed
plot \
    "< grep 'list-none-m,[0-9]*,1000,1' lab2_list.csv" using ($2):($8) \
	title 'mutex' with linespoints lc rgb 'blue', \
    "< grep -E 'list-none-s,[0-9]*,1000,1' lab2_list.csv" using ($2):($8) \
	title 'spin' with linespoints lc rgb 'orange'
