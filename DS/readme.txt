compile the bank_object first them the other 2 with pthread
then run 2 server with command 
./database_server 0
./database_server 1 <database server 0 ip>

on client side:
./database_client <database server ip>
