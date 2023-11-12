#!/bin/bash
make all
scp start_my_test_remote_jan.sh technikum-cluster:~/dev/bfs/
ssh technikum-cluster "chmod +x ~/dev/bfs/start_my_test_remote_jan.sh"
scp ./out/* technikum-cluster:~/dev/bfs/
ssh technikum-cluster "~/dev/bfs/start_my_test_remote_jan.sh"