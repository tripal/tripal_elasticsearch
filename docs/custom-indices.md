# Building Custom Indices
The Tripal Elasticsearch module allows you to index any tables from the public and Chado schema.
You can index all fields or a subset of fields from a table. You can select mapping types for each fields. 
Only fields that have a mapping type selected will get indexed and become searchable later.

Database table indexing also allows you to specify **tokenizer** and **token filters**. If you are not familiar with
these concepts, we recommend you select `standard` for tokenizer and `standard` for token filters. Below is an example:

![database indexing](../images/database-table-index.png) 

## Creating a new custom Index

You can create a new index by clicking on the create Index tab, or navigating to `http://[your-tripal-site-domain]/admin/tripal/extension/tripal_elasticsearch/indices/create`.  

For a custom index, you will need to:

* select the database table index type.
* Enter a unique name for the index.
* Specify Index settings for the Tokenizer and Token Filter (or leave the defaults)
* Decide if the index will be exposed to Cross-Site Search.
* Select a table.
* Select at least one field from the table to index.



**Next section: [Building Search Forms](/docs/forms.md)**

**[Return to Handbook Index](/docs)**