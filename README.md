# A_Database_Platform

In my professional career, I have interacted with databases as a user and as an administrator. My interactions with the front-end of these databases have sparked the creation of many questions I have about their back-end functionality:

- How is data stored?
	- What information in addition to the actual data is stored?
	- What is the format and structure of data files?
	- How are encryption and compression used in the storage and retrieval of data?
- How is data retrieved?
	- How is a SQL statement transformed into something with which data can be retrieved? Is it compiled? Is it parsed?
	- How do where clauses work?
	- How do joins work?

In an effort to learn about a database's back-end, I have started this project with the following goals:

1. Plan and implement a rough draft of how I think a database works including only the minimum functionality of table creation, data insertion, data updation, data deletion, and data selection
2. Use tests to ensure the program's reliability and to compare its performance against a known and trusted database
3. Use the results of the tests to consider a different program implementation and/or data file structure. Repeat implementation and testing as necessary until results are satisfactory.

This project is a learning experience. I have some ideas about how a database might work, so I am taking a stab in the dark, learning what works and what doesn't, all in the hopes that I can get close to the performance of a database like Oracle or SQL Server. 

# Planned Features
## Phase 1
- Table creation
- Data insertion
- Data updation
- Data deletion
- Data selection
## Phase 2
- Table column insertion
- Table column deletion
- Table deletion
- Table selection joins
- Table selection by groups and aggregate functions
## Phase 3
- Server setup
	- HTML and Javascript front-end with calls to the back-end C drivers for data retrieval and manipulation
