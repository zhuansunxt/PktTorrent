* Hashmap from [here](https://github.com/petewarden/c_hashmap).

For a quick experiment of a two-peer system, follow the below instructions.

./init_spiffy.sh

Initiate spiffy network environment.

./quick_start_1.sh

Launch peer 1.

Start a new terminal window(since logs are printed to the terminal), and type `./quick_start_2.sh` to launch peer 2.

Now two peers in the system are both launched. To start a downloading session on peer 1, type `GET user_file/user_a.chunks user_file/user_a.output` in peer 1's console.

This will start a downloading process on peer 1, which will request chunks that are owned by peer 2.
