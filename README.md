# LKM for Policy Fetcher
## Configure
* Build OPA server
 ```console
 git clone https://github.com/eBPFDevSecTools/eBPFsentinel
 cd eBPFsentinel/sentinel/Opa-Engine
 # Build OPA server
 sudo docker-compose -f ebpf-docker-compose.yml build
 ```
* Build LKM
 ```console
 cd ../../PolicyFetcher/LKM/
 make
 sudo cp ../app/dummy_kvstore.txt /boot/kvstore
 ```

 ## Execute
 * Start OPA server
 ```console
 sudo docker-compose -f ebpf-docker-compose.yml up -d
 ```
 * Configure LKM client
 	- Find server IP and modify `config.h` as described in the header file
 	- Configure, rebuild, and run LKM
	 ```console
	 make
	 sudo insmod lkm_http_request.ko
	 ```
	- Check with `sudo dmesg --follow`
## Cleanup
 * Stop LKM
   ```console
 	sudo rmmod lkm_http_request
   ```
 * Check with `sudo dmesg --follow`
 * Stop OPA server
   ```console
   sudo docker-compose -f ebpf-docker-compose.yml down
   ```