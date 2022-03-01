import pymongo
import socket
import json 

localIP     = socket.gethostbyname(socket.gethostname())
localPort   = 3001
bufferSize  = 4096

databaseName   = "netflow_data"
collectionName = "data"

def get_database(db_name):
    client = pymongo.MongoClient("mongodb://root:root@collector_mongodb:27017/")
    return client[db_name]

def get_collection(db, col_name):
    return db[col_name]

if __name__ == "__main__":
    print("After start")
    db = get_database(databaseName)
    collection = get_collection(db, collectionName)

    print("Connected to DB")

    socketServer = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

    # Bind to address and ip
    socketServer.bind((localIP, localPort))
    # Listen for incoming datagrams
    while(True):
        print("Listening...")
        data = socketServer.recvfrom(bufferSize)[0]
        try:
            dataJson = json.loads(data)
            print(data, dataJson)

            insertedIds = collection.insert_one(dataJson)

            print(f"Inserted {str(insertedIds).encode()} rows.")
        except json.JSONDecodeError:
            print(f"{data} is not in json fromat")
            



