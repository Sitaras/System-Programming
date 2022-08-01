#! /bin/bash
if (( $# < 1 ))
then
  echo "Program execution template: ./finder.sh [com gr org ...]"
  echo "Please, try again."
  exit -1
fi

tlds=() # array to save the tlds
counters=() # array of counters for the tlds

for tld in $(echo $@ | tr " " " ") # initialize the arrays
do
  tlds+=( $tld ) # add the name of tld from the command line
  counters+=( 0 ) # init the corresponding counter
done

filenames=`ls *.out` # take all .out files

for file in $(echo $filenames | tr "\n" "\n") # for every .out file
do
  while IFS= read -r line # read each .out file line by line
  do
      temp=`echo ${line##*.}` # store the tld and the appearances number (after separate them)
      tld=${temp% *} # store only the tld
      appearances=${temp#* }  # store only the appearances number of the corresponding tld

      for t in "${!tlds[@]}"; do # find the corresponding tld in to the array of tlds
       if [[ "${tlds[$t]}" == "${tld}" ]]; then
          counters[$t]=$(( counters[$t]+appearances )) # then go at the same index of counters array and increase the appearances number of the specific tld
          break
       fi
      done

    done < "$file"
done

# finally print these two arrays
for t in "${!tlds[@]}"; do
  echo -n "${tlds[$t]}"
  echo " ${counters[$t]}"
done
