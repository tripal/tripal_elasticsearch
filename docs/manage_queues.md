**Previous Section:** [Cross Site Searching](cross-site-search.md)

### Cancelling the Indexing Process
Deleting an index does not guarantee that the indexing stops. This is a limitation caused by Drupal's queue system where
they will keep executing if not explicitly deleted. Therefore, before deleting an index using Tripal Elasticsearch,
you should delete all queues that contain the name elasticsearch. To do so, visit `/admin/config/system/queue-ui`, check
all relevant queues and press the  `delete queues` button. After that, go to the manage indices page
(at `/admin/tripal/extension/tripal_elasticsearch/indices`) and delete the index.

**Next Section:** [System Resources](system-resources.md)
