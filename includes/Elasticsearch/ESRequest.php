<?php
/**
 * Provides RESTful HTTP request API.
 *
 * @file includes/Elasticsearch/ESRequest.php
 */

class ESRequest {

  public static $base_url = '';

  /**
   * Create a GET request.
   *
   * @param $url
   * @param null $data
   *
   * @return mixed
   */
  public static function get($url, $data = NULL) {
    return static::send('GET', $url, $data);
  }

  /**
   * Create a POST request.
   *
   * @param $url
   * @param null $data
   *
   * @return mixed
   */
  public static function post($url, $data = NULL) {
    return static::send('POST', $url, $data);
  }

  /**
   * Create a DELETE request.
   *
   * @param $url
   * @param null $data
   *
   * @return mixed
   */
  public static function delete($url, $data = NULL) {
    return static::send('DELETE', $url, $data);
  }

  /**
   * Create a PUT request.
   *
   * @param $url
   * @param null $data
   *
   * @return mixed
   */
  public static function put($url, $data = NULL) {
    return static::send('PUT', $url, $data);
  }

  /**
   * Create a curl HTTP request.
   *
   * @param string $method One of GET, PUT, POST or DELETE. Must be capitalized.
   * @param string $url The url to call. If base_url is set, this value will be
   *                    appended to base_url.
   * @param null|string|array $data
   *
   * @return mixed
   */
  public static function send($method, $url, $data = NULL) {
    global $conf;

    if (!empty(static::$base_url)) {
      $url = static::$base_url . $url;
    }

    $curl = curl_init();

    switch ($method) {
      case "POST":
        curl_setopt($curl, CURLOPT_POST, 1);

        if ($data) {
          curl_setopt($curl, CURLOPT_POSTFIELDS, $data);
        }
        break;
      case "DELETE":
        curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        if ($data) {
          curl_setopt($curl, CURLOPT_POSTFIELDS, $data);
        }
        break;
      case "PUT":
        curl_setopt($curl, CURLOPT_PUT, 1);
        break;
      default:
        if ($data) {
          $url = sprintf("%s?%s", $url, http_build_query($data));
        }
    }

    curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, TRUE);

    if(isset($conf['environment']) && in_array($conf['environment'], ['development', 'test', 'debug'])) {
      curl_setopt($curl, CURLOPT_SSL_VERIFYHOST, 0);
    }

    $result = curl_exec($curl);

    if($result === false) {
      throw new Exception(curl_error($curl));
    }

    curl_close($curl);

    return json_decode($result);
  }
}