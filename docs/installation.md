# Installation

After installing Elasticsearch and the Elasticsearch PHP library as highlighted in the Prerequisites section, you are ready to install Tripal Elasticsearch Drupal module.

This module can be installed using one of the following methods.

#### Option 1: Using Git (recommended)
- Navigate to your drupal's root directory
- Run the following commands
```shell
cd sites/all/modules
git clone https://github.com/tripal/tripal_elasticsearch.git
git checkout 7.x-2.0
drush en tripal_elasticsearch -y && drush updatedb
```
 
#### Option 2: Direct Download
- Download the latest release form the [releases page](https://github.com/tripal/tripal_elasticsearch/releases).
- Uncompress the file in your drupal-root-directory/sites/all/modules
- Enable the module from your drupal app **OR** run the following command
```shell
drush en tripal_elasticsearch -y && drush updatedb
```  

To learn more about installing Drupal modules, please visit the [Drupal documentation](https://www.drupal.org/node/895232).

## Connecting to Elasticsearch Servers
The Tripal Elasticsearch module allows connections to local and remote servers.  Your local connection is the server for your own site: you will be able to manage your indices and the details of this cluster.  Note that your Elasticsearch server need not be hosted alongside your Tripal site: local means that is the service that indexes and searches **your Tripal site**.  Remote connections allow you to connect to other websites and include their search results on your Tripal site.  Remote services are managed on their respective sites.

### Connect to your local Elastic cluster

Go to `http://[your-tripal-site-domain]/admin/tripal/extension/tripal_elasticsearch` and 
enter the host and port of your elastic cluster.  Make sure that the Server Type radio button is set to **local**. For example, the image below shows that
the Elastic server is running on the same host as the Tripal site, and the port is 9203.

![connect to elastic](images/elastic_search_connect.png)

#### Local Elasticsearch Server Health

This table provides feedback on the health of your connected local Elasticsearch Server.  If your connection is succesful, the Status column will be Green. For more information on the returned statistics, please see [the Elasticsearch documentation](https://www.elastic.co/guide/en/elasticsearch/reference/current/_cluster_health.html).

### Connect to a remote server

To add a new remote Elasticsearch server, select the Remote Server Type radio button,  fill out the server URL, Label, and Description, and press the Save Remote Host button. 

After connecting a remote server, it will be displayed in the remote server health table (example below).  If the connection is successful, the Status column will be green/Active and you can include this remote connection in your search interface.  If the status remains Red, ensure that you have the correct URL, and that your firewall is not blocking connections.  You may also edit and delete your remote servers using this table.

![remote server health](images/remote_server.png)
