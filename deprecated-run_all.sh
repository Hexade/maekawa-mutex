#./bin/sh

cd bin/
for i in {1..3}
do
    gnome-terminal -e "bash -c \"./server $i; exit; exec bash\""
done

for i in {1..7}
do
    gnome-terminal -e "bash -c \"./client $i; exit; exec bash\""
done
