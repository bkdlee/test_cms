var cms = {
	global: {
		init: function(){
			cms.user.deleteConfirm();
		}
	},
	user:{
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
								if ( data === true){
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