import os
import re
import os
import sys

query_gen = re.compile(r'^(Main: PIRClient query generation time:\s)(\d*.\d*\s)(s)$')

query_size = re.compile(r'^(Main: Query size:\s)(\d*\s)(bytes)$')

server_reply = re.compile(r'^(Main: PIRServer reply generation time:\s)(\d*.\d*\s)(s)$')

reply_size = re.compile(r'^(Main: Reply size:\s)(\d*\s)(bytes)$')

recover_time = re.compile(r'^(Main: PIRClient answer decode time:\s)(\d*.\d*\s)(s)$')

query_gen_list=[]
query_size_list=[]
server_reply_list=[]
reply_size_list=[]
recover_time_list=[]

for line in open(sys.argv[1]):
    match_str = re.match(query_gen, line)
    if match_str != None:
        query_gen_list.append(match_str.group(2))

    match_str = re.match(query_size, line)
    if match_str != None:
        query_size_list.append(match_str.group(2))
    
    match_str = re.match(server_reply, line)
    if match_str != None:
        server_reply_list.append(match_str.group(2))

    match_str = re.match(reply_size, line)
    if match_str != None:
        reply_size_list.append(match_str.group(2))

    match_str = re.match(recover_time, line)
    if match_str != None:
        recover_time_list.append(match_str.group(2))

print("query gen:")
for i in query_gen_list:
    print(i)

print("reply_size_list:")
for i in reply_size_list:
    print(i)

print("query_size_list:")
for i in query_size_list:
    print(i)

print("server_reply_list:")
for i in server_reply_list:
    print(i)

print("recover_time_list:")
for i in recover_time_list:
    print(i)