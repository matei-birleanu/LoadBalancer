# Load Balancer Implementation

**Author:** BIRLEANU TEODOR MATEI
**Year:** 2023  

---

## Overview

This project implements a **Load Balancer** using consistent hashing to manage servers and distribute data. The load balancer is designed to handle server addition, removal, and efficient key-value storage and retrieval across multiple servers.

---

## Key Features

### 1. **Consistent Hashing**
- The load balancer uses a consistent hashing algorithm to map keys and servers onto a circular hash ring.
- Each server has three replicas to ensure even data distribution.

### 2. **Server Management**
- **Adding Servers**:  
  - Servers are added to the hash ring along with their replicas.
  - Data is redistributed to maintain consistent hashing properties.
- **Removing Servers**:  
  - When a server is removed, its data is redistributed to the next server in the hash ring.

### 3. **Data Storage and Retrieval**
- **Storage**:
  - Keys are hashed and mapped to a server using consistent hashing.
  - Data is stored in the server's hashtable.
- **Retrieval**:
  - Keys are hashed and located on the appropriate server for retrieval.
  - If the key does not exist, `NULL` is returned.

### 4. **Dynamic Rebalancing**
- When servers are added or removed, the load balancer ensures data redistribution without disrupting the system.

---

## Core Components

### 1. **Hash Functions**
- `hash_function_servers`: Hashes server IDs to place them on the hash ring.
- `hash_function_key`: Hashes keys to determine their position on the ring.

### 2. **Binary Search**
- A binary search is used to efficiently locate the position of servers and keys on the hash ring.

### 3. **Server Replication**
- Each server is represented by three replicas on the hash ring for load balancing and fault tolerance.

### 4. **Hashtable Management**
- Each server has its own hashtable to store key-value pairs.

---

## API

### 1. **Initialization**
```c
load_balancer *init_load_balancer();
