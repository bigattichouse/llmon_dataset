# 🍋-llmon dataset
Open synthetic dataset for parsing syslog and journalctl output, created by analysis of open project repos. Intended to be used in https://github.com/bigattichouse/llmon to provide RAG and context information about log messages in realtime `journalctl` and `syslog` analysis.

# Primary purpose
Dataset will be used for Retrieval Augmented Generation (RAG) for the 🍋-LLmon system monitor.
Additionally, this dataset may be used to fine-tune an LLM to be better suited to processing linux syslog and journalctl entries and provide diagnostic opinions on how to optimize linux systems and post-mortem system failures.

# Data Updates
By separating the dataset from the main codebase, it will be possible to use `git` as a simple distribution mechanism for source updates.


# Design Philosophy
Reading and analyzing errors in system logs is not trivial and requires a lot of general knowledge about how software works.
Since most linux software is based on open source, why not use those project public git repos and anaylze the code base for instances of thrown errors and logged information. By having an LLM analyze all the individual models in the codebase, we can create a large knowledgebase of potential log messages and the original intent of those messages. Then, at run time, we can use RAG to locate relevant information and use that intermixed with the actual log messages to allow a small reasoning LLM to give us a summary of events and potential changes to be made.

In all things, I hoped to keep this tool as dead simple as possible - using bash and C (or C++) to run with as few external requirements as possible. This should follow the unix philosophy of a single small tool to do each job well. It's definitely helped me create the pipeline using AI - since the LLMs can focus on one discrete task at a time.

# Build Process
You should not need to build projects (apache/httpd), they should all be available in the `projects/` directory. The build process is only included in case you wish to add your own project to the dataset and initiate a pull request to have your project accessible to everyone. It's also nice if people want to help improve the build logic and pipeline when such tools are created.

# Branches

Thinking of maybe doing branches for each major project Apache, Mysql, Sendmail, etc.  Then I could do packages like LAMP (Apache/Mysql/PHP) as branches.  This way you could pull/merge what you want without needing everything else.
