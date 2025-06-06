## To conduct the experiments to retrieve the measurements, I followed these steps: 

- Within the /code directory execute 'make all'

- Started the slurpe-5 program with fullpe and the address of where the probe program is running. For example:  
   ./slurpe-5 fullpe <probe_program_addr> <probe_program_addr> 

- Use the script command on the terminal where the probe program is being executed. This records everything printed on the terminal, the path of the output can also be specified (all my output files can be seen within the /data directory).  For example: script <file_path>

- Run the probe program from the <probe_program_addr>, while also specifying the address where slurpe-5 is being run from and the payload size for that experiment. For example: 
  ./slurpe-probe <target_addr> <payload_size> 

- The received packets information on is being printed on the terminal and recorded as the probe programs output.  

- After the probe program has finished (sent all packets), then use ‘exit’ so the data outputted by the probe program is recorded.  


## File descriptions:
- /code direcrory
  - slurpe-probe.c the probe packet program
  - UdpSocket.c allows for the creation of UDP sockets
  - Makefile used to make the .c files executable
- /data directory
  - /images directory which stores the images of the graphs
  - output_big.txt which has the output when the experiment was run using big packets
  - output_small.txt which has the output when the experimetn was run using the small packets
  - plot_data.ipynb which is the Jupyter Notebook where all the graphs are produced and some calculations are performed
  - process_data.py which is a python file that has some functions that are used to process the data which is outputted from the probe program
  - requirements.txt which are all the requirements needed for the python and Jupyter Notebook files
