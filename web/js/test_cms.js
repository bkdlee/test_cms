var cms = {
	global: {
		init: function(){
			cms.user.init();
		}
	},
	user:{
		init: function(){
			cms.user.formValidate();
			cms.user.deleteConfirm();
		},
		formValidate: function(){
			$("#edit_user").validate({
				rules:{
					password:{
						minlength: 8,
                        validpassword: true
					},
					password2:{
						minlength: 8,
						equalTo: "#password"
					}
				}
			});
			$.validator.addMethod("validpassword", function(value, element) {
                            var result = false;
                            var m = value.match(/\d+/g);
                            var num = '';
                            $.each(m, function(){
                                num += m;
                            });
                            if (num.length >= 2){
                                result = true;
                            }
                            return result;
			}, "password must be contain a minimum of 2 numbers");
		},
		deleteConfirm: function(){
			$(".glyphicon-remove").each(function(){
				var url = $(this).attr("href");
				$(this).attr("href", "#");
				$(this).attr("rel", url);
			});
			$(".glyphicon-remove").click(function(){
				var url = $(this).attr("rel");
				$("#dialog").html("These user will be permanently deleted and cannot be recovered. Are you sure?")
				.dialog({
					title: "Confirm",
					modal: true,
					resizable: false,
					buttons:{
						"Delete user": function(){
							$.get(url, function(data){
								if ( data === "T"){
									location.reload();
								} else {
									alert("error");
								}
							});
						},
						"Cancel": function(){
							$( this ).dialog( "close" );
						}
					}
				});
			});
		}
	}
}