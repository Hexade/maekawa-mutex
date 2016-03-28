#./bin/sh

for i in {1..3}
do
    pkill -f "server $i"
done

for i in {1..7}
do
    pkill -f "client $i"
done
