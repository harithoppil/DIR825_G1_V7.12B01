
//加载初始化
function uiOnload(){
	
}


//错误处理函数
function dealWithError(){
	if (G_Error != 1){ 
		return false;
	}
	var arrayHint = [];
	dealErrorMsg(arrayHint, G_Error_Msg);
}
//监听加载与错误处理函数
addListeners(uiOnload, dealWithError);