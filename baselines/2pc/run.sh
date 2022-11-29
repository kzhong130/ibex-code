for i in {1..10}
do
    echo "run $i times"
    ./bin/test_hist 1 12345 15 100 & ./bin/test_hist 2 12345 15 100
done

for i in {1..10}
do
    echo "run $i times"
    ./bin/test_hist 1 12345 15 500 & ./bin/test_hist 2 12345 15 500
done

for i in {1..10}
do
    echo "run $i times"
    ./bin/test_hist 1 12345 16 100 & ./bin/test_hist 2 12345 16 100
done

for i in {1..10}
do
    echo "run $i times"
    ./bin/test_hist 1 12345 16 500 & ./bin/test_hist 2 12345 16 500
done

for i in {1..10}
do
    echo "run $i times"
    ./bin/test_hist 1 12345 17 100 & ./bin/test_hist 2 12345 17 100
done

for i in {1..10}
do
    echo "run $i times"
    ./bin/test_hist 1 12345 17 500 & ./bin/test_hist 2 12345 17 500
done