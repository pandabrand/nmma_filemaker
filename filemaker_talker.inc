<?php
  class FileMakerTalker {
    protected $api_server;
    protected $api_database;
    protected $api_layout;
    protected $api_auth;

    public function __construct() {
      $this->api_server = variable_get('api_server');
      $this->api_database = variable_get('api_db');
      $this->api_layout = variable_get('api_layout');
      $this->api_auth = variable_get('api_auth');
    }

    public function getToken() {

      $request = new HttpRequest();
      $request->setUrl("https://$this->api_server/fmi/data/v1/databases/$this->api_database/sessions");
      $request->setMethod(HTTP_METH_POST);

      $request->setHeaders(array(
        'Postman-Token' => '9f053441-448b-4b2d-9c2b-ecb56d7eb7c6',
        'Cache-Control' => 'no-cache',
        'Authorization' => $this->api_auth,
        'Content-Type' => 'application/json'
      ));

      $request->setBody('{}');

      try {
        $response = $request->send();
        $body = $response->getBody();
        watchdog('migrate_logging', '<pre>' . $body . '</pre>');
      } catch (HttpException $ex) {
        watchdog('migrate_logging', '<pre>error: ' . $ex . '</pre>');
        echo $ex;
      }

      return $body;
    }
  }
  class FileMakerListJSON extends MigrateListJSON {
    /**
     * The default implementation assumes the IDs are top-level array elements,
     * but the array elements are the data items - we need to look inside them
     * for the IDs.
     */
    protected function getIDsFromJSON(array $data) {
      $ids = array();
      foreach ($data['response']['data'] as $item) {
        $ids[] = $item['recordId'];
      }
      return $ids;
    }

    /**
   * If this is the case then the count will be off as well and we will have to overriden as well.
   */
  public function computeCount() {
    $count = 0;
    if (empty($this->httpOptions)) {
      $json = file_get_contents($this->listUrl);
    }
    else {
      $response = drupal_http_request($this->listUrl, $this->httpOptions);
      $json = $response->data;
    }
    if ($json) {
      $data = drupal_json_decode($json);
      if ($data) {
        $count = count($data['response']['data']);
      }
    }
    return $count;
  }

  }

  class FileMakerItemJSON extends MigrateItemJSON {
    protected $data = array();
    /**
     * Two problems with the default getItem() - it reads and parses the JSON on
     * each call, which we don't want to do for multiple items in the file; and,
     * we need to retrieve a given item from the file according to its 'id' element.
     */
    public function getItem($id) {
      // We cache the parsed JSON at $this->data.
      if (empty($this->data)) {
        $data = $this->loadJSONUrl($this->itemUrl);
        if ($data) {
          // Let's index the array by the ID for easy retrieval.

          foreach ($data as $item) {
            $this->data[$item->id] = $item;
          }
        }
        else {
          // Error-handling here....
        }
      }
      // Return the requested item
      if (isset($this->data[$id])) {
        return $this->data[$id];
      }
      else {
        return NULL;
      }
    }
  }