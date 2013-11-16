<?php



class rest{
    /**
     * The HTTP request method used.
     * @var str
     */
    var $method = 'GET';
    /**
     * The HTTP request data sent (if any).
     * @var str
     */
    var $requestData = NULL;
    /**
    * The URL extension stripped off of the request URL
    * @var str
    */
    var $extension = NULL;
    
    /**
     * Execute the request.
     */
    function exec() {
        switch ($this->method) {
            case 'GET':
                $result = $this->get();
                break;
        }   
        return $result;
    }
    
    function connect() {
        if (isset($_SERVER['PHP_AUTH_USER']) && isset($_SERVER['PHP_AUTH_PW'])) {
            $email = $_SERVER['PHP_AUTH_USER'];
            $password = $_SERVER['PHP_AUTH_PW'];
            if ( !check_login_details($email, $password, 1)){
                $this->unauthorized();
                 exit();
            }           
        } else {
            $this->unauthorized();
            exit();
        }
    }
    
    
    function get() {
        $rows = [];
        $i = 0;
        include('../src/model/model.php');

        $model = &new model();
        $rows = $model->get_user_list();
        $users = array();
        foreach($rows as $i=>$row){
            $users[$i]['first_name'] = $row['first_name'];
            $users[$i]['last_name'] = $row['last_name'];
            $users[$i]['email'] = $row['email'];
            $users[$i]['active'] = $row['active'];
            $users[$i]['facebook_id'] = $row['facebook_id'];
            $users[$i]['created_date'] = $row['created_date'];
            
        }
        return json_encode($users);
        
    }

    /**
     * Send a HTTP 201 response header.
     */
    function created($url = FALSE) {
        header('HTTP/1.0 201 Created');
        if ($url) {
            header('Location: '.$url);   
        }
    }
    
    /**
     * Send a HTTP 204 response header.
     */
    function noContent() {
        header('HTTP/1.0 204 No Content');
    }
    
    /**
     * Send a HTTP 400 response header.
     */
    function badRequest() {
        header('HTTP/1.0 400 Bad Request');
    }
    
    /**
     * Send a HTTP 401 response header.
     */
    function unauthorized($realm = 'TEST CMS') {
        header('WWW-Authenticate: Basic realm="'.$realm.'"');
        header('HTTP/1.0 401 Unauthorized');
    }
    
    /**
     * Send a HTTP 404 response header.
     */
    function notFound() {
        header('HTTP/1.0 404 Not Found');
    }
    
    /**
     * Send a HTTP 405 response header.
     */
    function methodNotAllowed($allowed = 'GET, HEAD') {
        header('HTTP/1.0 405 Method Not Allowed');
        header('Allow: '.$allowed);
    }
    
    /**
     * Send a HTTP 406 response header.
     */
    function notAcceptable() {
        header('HTTP/1.0 406 Not Acceptable');
        echo join(', ', array_keys($this->config['renderers']));
    }
    
    /**
     * Send a HTTP 411 response header.
     */
    function lengthRequired() {
        header('HTTP/1.0 411 Length Required');
    }
    
    /**
     * Send a HTTP 500 response header.
     */
    function internalServerError() {
        header('HTTP/1.0 500 Internal Server Error');
    }
}
?>

