<?php

class NMMACollectionMigration extends Migration {
  public $api_server;
  public $api_database;
  public $api_layout;
  public $api_auth;

  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t('Import Collection from FileMaker Cloud');
    $this->defaultLanguage = 'en';
    $this->api_server = variable_get('api_server');
    $this->api_database = variable_get('api_db');
    $this->api_layout = variable_get('api_layout');
    $this->api_auth = variable_get('api_auth');

    $fields = array();
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