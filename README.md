# SyncText: A CRDT-Based Collaborative Text Editor

This project is a real-time, multi-user collaborative text editor that simulates the functionality of Google Docs. It is built for the CS69201 (Computing Lab) course.

The system is fully decentralized, lock-free, and serverless. It uses a **Conflict-Free Replicated Data Type (CRDT)** model based on the **Last-Writer-Wins (LWW)** strategy to ensure all users' documents eventually converge to the same state.

* **User Discovery:** Implemented using a lock-free shared memory registry.
* **Communication:** Implemented using POSIX message queues for peer-to-peer broadcasting.

---

## 🖥️ Platform

This project is built for a specific Linux environment as per the project guidelines.

* **OS:** Ubuntu 20.04 (or a compatible Linux distribution)
* **Compiler:** g++ 9.4.0 (or any C++17 compatible compiler)
* **Important:** This code **will not** compile on macOS or Windows, as it requires Linux-specific features like POSIX message queues (`mqueue.h`).

---

## 📦 Dependencies

The project requires the following standard Linux libraries:
* `pthread`: Required for C++ threading (`std::thread`) and atomic operations.
* `rt` (Real-Time Library): Required for POSIX shared memory (`shm_open`) and message queues (`mq_open`).

---

## ⚙️ Compilation Instructions

The project uses a `Makefile` that finds all source files in the `src/` directory, compiles them into an `obj/` directory, and links the final executable.

To compile the project, run:
```bash
make
```
This will create the editor executable in the root directory.

To clean the build files (removes the `obj/` directory and the editor executable):
```bash
make clean
```

---

## Execution Instructions



Execution Instructions
To run the program, you must open a separate terminal for each user.

Pass a unique user_id as a command-line argument for each instance.

Terminal 1:

```bash
./editor user_1
```

Terminal 2:

```bash
./editor user_2
```

Terminal 3:

```bash
./editor user_3
```

The program will automatically create a local document for each user (e.g., user_1_doc.txt).


## How to Test
You can test the system by running 2-3 users and editing their respective local files (e.g., user_1_doc.txt) with any external text editor (like nano, vim, or gedit).

The system batches operations, so you may need to save 5 times (or make 5 changes) to trigger a broadcast and merge cycle.

### Test 1: Non-Conflicting Edits

User 1: Open user_1_doc.txt and change Line 0. Save 5 times.

User 2: Open user_2_doc.txt and change Line 1. Save 5 times.

Result: After the merge, observe both terminals. Both user_1 and user_2's documents will converge to show the changes from both users on lines 0 and 1.

### Test 2: Conflicting Edits (LWW)

User 1: Open user_1_doc.txt and change Line 0 to Hello from User 1. Save 5 times.

User 2: Open user_2_doc.txt and change the same Line 0 to Hello from User 2. Save 5 times, ensuring you save after User 1.

Result: Observe both terminals. Both documents will converge to show Hello from User 2. The Last-Writer-Wins (LWW) rule gives precedence to the edit with the latest timestamp.

### Test 3: LWW Tiebreaker

If two users (e.g., user_1 and user_2) make a conflicting change at the exact same timestamp.

Result: The conflict will be resolved by the user_id. The edit from user_1 will win because "user_1" is alphabetically smaller than "user_2".

To clean the build files (removes the `obj/` directory and the editor executable):
