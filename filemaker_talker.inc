<?php
  class FileMakerTalker {
    protected $api_server;
    protected $api_database;
    protected $api_layout;
    protected $api_auth;
    protected $json_file;

    public function __construct() {
      $this->api_server = variable_get('api_server');
      $this->api_database = variable_get('api_db');
      $this->api_layout = variable_get('api_layout');
      $this->api_auth = variable_get('api_auth');
      $this->json_file = '';
    }

    public function auth() {
      $url = "https://$this->api_server/fmi/data/v1/databases/$this->api_database/sessions";
      $method = 'POST';

      $headers = array(
        'Cache-Control' => 'no-cache',
        'Authorization' => $this->api_auth,
        'Content-Type' => 'application/json'
      );
      // Migration::displayMexssage(print_r($headers, true));

      $result = drupal_http_request($url, array(
        'method' => $method,
        'headers' => $headers
      ));

      $body = null;
      if($result->status_message == 'OK') {
        $body = json_decode($result->data);
      }

      return $body;
    }

    function get_token() {
      if ( $cached_token = cache_get('fm_token', 'cache' ) ) {
        $token = $cached_token->data;
      } else {
        $auth = $this->auth();
        $token = $auth->response->token;
        cache_set('fm_token', $token, 'cache', time() + 30);
      }

      return $token;
    }

    function get_items() {
      $url = "https://$this->api_server/fmi/data/v1/databases/$this->api_database/layouts/$this->api_layout/_find";
      $method = 'POST';

      $headers = array(
        'Cache-Control' => 'no-cache',
        'Authorization' => 'Bearer ' . $this->get_token(),
        'Content-Type' => 'application/json'
      );

      $query_string = '{"query": [{"Approved_for_Web": "=Web","Main": "=Main"}],"sort": [{"fieldName": "title","sortOrder": "ascend"}]}';

      $http_options = array(
        'method' => $method,
        'headers' => $headers,
        'data' => $query_string
      );

      // Migration::displayMessage( print_r( $http_options, true ) );
      $result = drupal_http_request($url, $http_options);

      if($result->status_message == 'OK') {
        $body = $result->data;
        $timestamp = date("Y-m-d_H:i:s");
        $ft_name = $timestamp . '-filetalker.json';
        $this->json_file = file_save_data( $body, 'public://' . $ft_name );
      } else {
        Migration::displayMessage($result->status_message);
        Migration::displayMessage($result->data);
      }
      return $this->json_file;
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
      $records = $data['response']['data'];
      // Migration::displayMessage( print_r( $records, true ) );

      foreach ($records as $item) {
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
          $response_data = $data->response->data;
          // Migration::displayMessage(print_r($response_data, true));
          foreach ($response_data as $item) {
            $this->data[$item->recordId] = $item;
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

  class MigrateNMMAImageFieldHandler extends MigrateFieldHandler {
    public function __construct() {
      // This handler will take care of all the simple core field types
      $this->registerTypes(array('image'));
    }

    public function prepare($entity, array $field_info, array $instance, array $values) {
      $migration = Migration::currentMigration();

      Migration::displayMessage('values: ' . print_r($entity, true));
      $arguments = (isset($values['arguments']))? $values['arguments']: array();
      // In Drupal 7, field values are indexed by language, which can be specified
      // at various levels - this member implemented in MigrateFieldHandler works
      // out which language to use, applying appropriate defaults
      $language = $this->getFieldLanguage($entity, $field_info, $arguments);
      // Setup the standard Field API array for saving.
      $delta = 0;
      // foreach ($values as $value) {
        $return[$language][$delta]['fid'] = $values->fid;
      //   $delta++;
      // }
      if (!isset($return)) {
        $return = NULL;
      }
      return $return;
    }
  }