#Step 0 - cleanup and prepare to load a project 

The idea is that this build should be as close to idempotent as possible, I should be able to run it multiple times and get the same output. While the individual non-deterministic decisions (reasons, recommendations) may change due to LLM temperature, I should see the same general content each time. I could dial down the temp, but I worry the affect would be less useful reasoning.

