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
						minlength: 8
					},
					password2:{
						minlength: 8,
						equalTo: "#password"
					}
				}
			});
			$.validator.addMethod("validpassword", function(value, element) {
    			return this.optional(element) || /^.*(?=.{8,})(?=.*[a-z])(?=.*[A-Z])(?=.*[\d])(?=.*[\W]).*$/.test(value);
			}, "The password must contain a minimum of one lower case character," + " one upper case character, one digit and one special character..");
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