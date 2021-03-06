<?php
/**
 * Provides a response interface for Http Ajax requests.
 *
 * @file includes/Elasticsearch/ESResponse.php
 */

class ESResponse {

  /**
   * Return a 200 OK response.
   *
   * @param mixed $data Array or string of response data.
   *
   * @return bool|array|object
   */
  public static function success($data, $print = TRUE) {
    if ($print) {
      static::setHeaders(200);

      print json_encode([
        'data' => $data,
        'error' => FALSE,
      ]);
    }
    else {
      // Convert to object and return
      return json_decode(json_encode([
        'data' => $data,
        'error' => FALSE,
      ]));
    }

    return TRUE;
  }

  /**
   * Return a 401 Forbidden response.
   *
   * @return bool
   */
  public static function forbidden() {
    static::setHeaders(401);

    print json_encode([
      'data' => '401 Forbidden',
      'error' => TRUE,
    ]);

    return TRUE;
  }

  /**
   * Return Errors.
   * 422 Unprocessable Entity response by default.
   *
   * @param mixed $data Array or string of error message.
   *
   * @return bool
   */
  public static function error($data, $code = 422) {
    static::setHeaders($code);

    print json_encode([
      'data' => $data,
      'error' => TRUE,
    ]);

    return TRUE;
  }

  /**
   * Setup response headers.
   *
   * @param int $code
   *
   * @return void
   */
  protected static function setHeaders($code = 200) {
    // All our responses our going to be in JSON Format
    drupal_add_http_header('Content-Type', 'application/ld+json');
    // Allow other sites to request results from us using AJAX directly
    drupal_add_http_header('Access-Control-Allow-Origin', '*');
    drupal_add_http_header('Access-Control-Allow-Headers', 'Accept, Content-Type, Authorization, X-Requested-With');
    // Set the response code
    http_response_code($code);
  }
}