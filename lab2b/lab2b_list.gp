#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
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
    "< grep -E 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'mutex' with linespoints lc rgb 'blue', \
    "< grep -E 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'spin' with linespoints lc rgb 'orange'


# plot of wait-for-lock time and avg time per operation against number of competing threads
set title "List2b-2: Wait-For-Lock Time and Operation Time vs. Number of Threads"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Time (ns)"
set logscale y 10
set output 'lab2b_2.png'
set key right top

# grep out only needed
plot \
    "< grep -E 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'wait-for-lock time' with linespoints lc rgb 'red', \
    "< grep -E 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'avg operation time' with linespoints lc rgb 'green'

set title "List2b-3: Synchronization Method vs. # of Iterations that run without failure"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
# note that unsuccessful runs should have produced no output
plot \
    "< grep 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'none' with points lc rgb 'green', \
    "< grep 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'mutex' with points lc rgb 'blue', \
	"< grep 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'spin' with points lc rgb 'orange'

set title "List2b-4: Performance of Mutex Synchronization with Different # of Lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Aggregated Throughput"
set logscale y 10
set output 'lab2b_4.png'
# note that unsuccessful runs should have produced no output
plot \
    "< grep 'list-none-m,[0-9]*,[0-9]*,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=1' with linespoints lc rgb 'green', \
    "< grep 'list-none-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=4' with linespoints lc rgb 'blue', \
	"< grep 'list-none-m,[0-9]*,[0-9]*,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=8' with linespoints lc rgb 'red', \
	"< grep 'list-none-m,[0-9]*,[0-9]*,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=16' with linespoints lc rgb 'orange'

set title "List2b-5: Performance of Spinlock Synchronization with Different # of Lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Aggregated Throughput"
set logscale y 10
set output 'lab2b_5.png'
# note that unsuccessful runs should have produced no output
plot \
    "< grep 'list-none-s,[0-9]*,[0-9]*,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=1' with linespoints lc rgb 'green', \
    "< grep 'list-none-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=4' with linespoints lc rgb 'blue', \
	"< grep 'list-none-s,[0-9]*,[0-9]*,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=8' with linespoints lc rgb 'red', \
	"< grep 'list-none-s,[0-9]*,[0-9]*,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=16' with linespoints lc rgb 'orange'
