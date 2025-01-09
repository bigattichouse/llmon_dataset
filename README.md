# llmon
Open synthetic dataset for parsing syslog and journalctl output, created by analysis of open project repos.

# Primary purpose
Dataset will be used for Retrieval Augmented Generation (RAG) for the 🍋-LLmon system monitor.
Additionally, this dataset will be used to fine-tune an LLM to be better suited to processing linux syslog and journalctl entries and provide diagnostic opinions on how to optimize linux systems and post-mortem system failures.

# Design Philosophy
Reading and analyzing errors in system logs is not trivial and requires a lot of general knowledge about how software works.
Since most linux software is based on open source, why not use those project public git repos and anaylze the code base for instances of thrown errors and logged information. By having an LLM analyze all the individual models in the codebase, we can create a large knowledgebase of potential log messages and the original intent of those messages. Then, at run time, we can use RAG to locate relevant information and use that intermixed with the actual log messages to allow a small reasoning LLM to give us a summary of events and potential changes to be made.


