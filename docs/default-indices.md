# Default Indices
Tripal Elasticsearch supports a few default indices out of the box that makes it easy for a quick start.

## Website Nodes Index
The website index is a preconfigured index that is responsible for Drupal nodes (pages and other default drupal content).
It also comes with a preset form that can be used in place of the drupal default search block.

To start using this index, navigate to `Admin -> Tripal -> Extensions -> Tripal Elasticsearch -> Indices Management -> Create Index` 
and choose "Website Nodes" from the dropdown list, and complete & submit the form.

You can monitor the progress of the indexing job using the progress tracker.

Next, you'll need to activate the `Tripal Elasticsearch website search box` block. Learn more about [Drupal Blocks](https://www.drupal.org/docs/8/core/modules/block/overview).

## Tripal 3 Entities Index
Tripal Elasticsearch also provides a default index for Tripal 3 entities. To activate the index, follow the same procedure
as the website nodes index above, selecting "Entities" from the dropdown list when creating the index.

# Warning!

Before running the indexer, please make sure that your content types have the correct permissions. The indexer is only allowed to access content as an anonymous user. This prevents it from indexing private content that only administrators or authenticated users should be able to access.

### Create, Update and Delete Pages

Whenever you create a new page, update existing pages or delete pages, indexing jobs will automatically 
be added to the cron queues to reflect the event on the website nodes and entities indices. When the indexing jobs get
executed depends on how you configure cron jobs on your Tripal site. If you need the updated indexing process to start 
immediately, you can always launch your cron jobs manually by going to 
`http://[your-tripal-site-domain]/admin/config/system/cron`. If you have added the cron jobs into the crontab file, then 
no extra work needs to be done.

## Gene Search Index (Tripal Only)
This index provides a more customized search experience by extracting gene information such as annotations and blast hit descriptions
from a Chado database. This index is dependent on Tripal (2 or 3). It also requires availability of the following tables (which are available by default with any Tripal site):
- `chado.feature`
- `chado.blast_hit_data`
- `chado.feature_cvterm`
- `chado.cvterm`
- `chado.dbxref`
- `chado.db`
- `chado.cv`

If this setup is satisfied, you can run the indexing job in the same manner as other default indices.


**Next section: [Building Custom Indices](/docs/custom-indices.md)**

**[Return to Handbook Index](/docs)**
