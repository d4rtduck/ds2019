compile and run bank_object.c first to initialize the demo database
compile using:
gcc -o database_server -pthread database_server.c
gcc -o database_client -pthread database_client.c

then run 2 server with command 
./database_server 0
./database_server 1 <database server 0 ip, or 127.0.0.1>

on client side:
./database_client <database server ip, or 127.0.0.1>

Client input to test: QUERY <user_id> <amount_of_money>
Example: "QUERY 100 5000" (doesn't include quotation)
