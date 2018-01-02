**Previous section:  [](cross-site-search.md)**


# Running Indexing Queues
Before Tripal Elasticsearch can index your site's content, we have to setup cron jobs to process our queues.
Since indexing jobs can take some time to complete, we deploy the jobs in separate queues and utilize them to perform
concurrent indexing in the background.

Tripal Elasticsearch provides 11 queues that can run concurrently (in parallel). The queues can be activated using [cron](http://www.nncron.ru/help/EN/working/cron-format.htm)

- Open crontab using the following command:
```shell
crontab -e
```
- Paste the following lines. Make sure you change `/path/to/your/drupal/root` to your drupal root path.
```shell
*/5 * * * * drush cron-run queue_elasticsearch_dispatcher --options=thread=1 --root=/path/to/your/drupal/root
*/5 * * * * drush cron-run queue_elasticsearch_queue_1 --options=thread=2 --root=/path/to/your/drupal/root
*/5 * * * * drush cron-run queue_elasticsearch_queue_2 --options=thread=3 --root=/path/to/your/drupal/root
*/5 * * * * drush cron-run queue_elasticsearch_queue_3 --options=thread=4 --root=/path/to/your/drupal/root
*/5 * * * * drush cron-run queue_elasticsearch_queue_4 --options=thread=5 --root=/path/to/your/drupal/root
*/5 * * * * drush cron-run queue_elasticsearch_queue_5 --options=thread=6 --root=/path/to/your/drupal/root
*/5 * * * * drush cron-run queue_elasticsearch_queue_6 --options=thread=7 --root=/path/to/your/drupal/root
*/5 * * * * drush cron-run queue_elasticsearch_queue_7 --options=thread=8 --root=/path/to/your/drupal/root
*/5 * * * * drush cron-run queue_elasticsearch_queue_8 --options=thread=9 --root=/path/to/your/drupal/root
*/5 * * * * drush cron-run queue_elasticsearch_queue_9 --options=thread=10 --root=/path/to/your/drupal/root
*/5 * * * * drush cron-run queue_elasticsearch_queue_10 --options=thread=11 --root=/path/to/your/drupal/root
```

## Obtain cron job names

Go to `http://[your-tripal-site-domain]/admin/config/system/cron`. On this page, you will find
the 10 cron queues created by the Tripal Elasticsearch module. Hover the mouse over the **Run** link on the
right of each cron queue. At the bottom of the page, the URL will display. The cron job name is within
the URL. For example, the cron job name is `queue_elasticsearch_queue_1` in the example below:

![cron job name](../images/get-cron-name.png)