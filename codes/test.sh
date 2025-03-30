for i in {1..20}; do
  time ./nbtc 127.0.0.1 6001 < 2000.byte 1>/tmp/kq/"$i".txt 2&>1 &
  # echo "client-$i" | ./tcp-client "test" &
done;
