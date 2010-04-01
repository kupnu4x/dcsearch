function fixPNG(element){
    if(/MSIE (5\.5|6).+Win/.test(navigator.userAgent)){
        var src;        
        if(element.tagName=='IMG' || element.tagName=='INPUT'){
            if(/\.png$/.test(element.src)){
                src = element.src;
                element.src = "img/blank.gif";
            }
        }else{
            src = element.currentStyle.backgroundImage.match(/url\("(.+\.png)"\)/i)
            if(src){   
                src = src[1];
                element.runtimeStyle.backgroundImage="none";
            }
        }
        if(src)element.runtimeStyle.filter = "progid:DXImageTransform.Microsoft.AlphaImageLoader(src='" + src + "')";
    }
}