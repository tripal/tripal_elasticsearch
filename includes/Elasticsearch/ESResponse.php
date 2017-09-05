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
   * @return bool
   */
  public static function success($data) {
    static::setHeaders(200);

    print json_encode([
      'data' => $data,
      'error' => FALSE,
    ]);

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
    drupal_add_http_header('Content-Type', 'application/ld+json');
    http_response_code($code);
  }
}