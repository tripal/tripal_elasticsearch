# Requirements
* Elasticsearch
* Elasticsearch-PHP library

### Install Elasticsearch-PHP library
* Create a folder named `elasticsearch-php` within your drupal 'sites/all/libraries' directory.
* Move into the `sites/all/libraries/elasticsearch-php` and create a `composer.json` file with
the following content:

```
{
    "require": {
        "elasticsearch/elasticsearch": "~5.0"
    }
}
```
* Run the following command to install the library:

```
curl -s http://getcomposer.org/installer | php
php composer.phar install --no-dev
```

For more details, go to https://www.elastic.co/guide/en/elasticsearch/client/php-api/current/_quickstart.html.