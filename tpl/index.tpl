<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html;charset=utf-8" />
<meta name="Description" content="" />
<title></title>
<style type="text/css">
    body, div{
        margin:0;
        padding:0;
    }
    img{
        border:0;
    }
    #top{
        background-color: #000;
        height:50px;
        padding-top:5px;
    }
    #logo{
        font-family: Arial;
        font-size: 38px;
        color:#fff;
        margin:25px 0 0 25px;
    }
    #logo a{
        text-decoration: none;
        color:#fff;
    }
    #top form{
        display: inline;
        margin-left:25px;
        position:relative;
        top:-5px;
    }
    #q{
        width:500px;
    }
    #results{
        font-family: Arial;
        font-size: 12px;
        line-height: 24px;
        padding:10px 10px 10px 25px;
    }
    .result{
        margin: 10px 0 10px 0;
        padding : 0 0 0 42px;
        line-height:18px;
    }
    .result img.type{
        float:left;
        margin: 2px -32px 0 -42px;
    }
    .result img.magnet{
        vertical-align: top;
        position:relative;
        top:1px;
    }
    #footer{
        border-top:1px solid #000;
    }
    #pagination{
        float:left;
        font-family: Arial;
        font-size:12px;
        padding:10px 10px 10px 25px;
    }
    #pagination a{
        color:#0000ee;
    }
    .ctrlhint{
        color:#999;
        font-size: 13px;
    }
    #copyright{
        float:right;
        font-family: Arial;
        font-size: 10px;
        margin:3px 10px 0 0;
        text-align:right;
    }
    #copyright img{
        vertical-align: top;
    }
    .clear{
        clear:both;
        margin:-10px;
    }
</style>
{{if pagination}}
    {{if prevlink}}<link rel="prev" href="{{prevlink}}" id="prevlink" />{{end}}
    {{if nextlink}}<link rel="next" href="{{nextlink}}" id="nextlink" />{{end}}
    <script type="text/javascript">
        function ctrlMove(e){
            var key = e.keyCode?e.keyCode:(e.which?e.which:null);
            var link;
            switch(key){
                case 0x25:
                    link = document.getElementById ('prevlink');
                    break;
                case 0x27:
                    link = document.getElementById ('nextlink');
                    break;
            }
            if(link && link.href) document.location.href = link.href;
        }
        document.onkeydown = ctrlMove;
    </script>
{{end}}
</head>
<body>
    <div id="top">
        <span id="logo"><a href="?">DCSearch</a></span>
        <form method="get">
            <input type="text" name="q" id="q" value="{{query}}" />
            <input type="submit" value="search" />
        </form>
    </div>
    <div id="results">
        {{if nofound}}
        Sorry, no matches found
        {{end}}
        {{begin results}}
            <div class="result">
            {{if rowtype_tth}}
                <img src="img/tth.png" class="type" alt="TTH" /> //{{nick}}/{{fullpath}}/{{name}}.{{extension}}
                <!--[<a href="dchub://{{nick}}@172.16.22.1:4111/{{fullpath}}/{{name}}">link</a>]--><br/>
                <small>Size: {{size}}, TTH: <b>{{tth}}</b> <a href="{{magnet}}"><img src="img/magnet.png" class="magnet" alt="magnet link" /></a> </small><br/>
            {{end}}
            {{if rowtype_dir}}
                <img src="img/folder.png" class="type" alt="dir" /> //{{nick}}/{{fullpath}}/<b>{{name}}</b>/<br/>
                <small><i>directory</i></small>
            {{end}}
            {{if rowtype_file}}
                <img src="img/{{type}}.png" class="type" alt="file" /> //{{nick}}/{{fullpath}}/<b>{{name}}.{{extension}}</b>
                <!--[<a href="dchub://{{nick}}@172.16.22.1:4111/{{fullpath}}/{{name}}">link</a>]--><br/>
                <small>Size: {{size}}, TTH: {{tth}} <a href="{{magnet}}"><img src="img/magnet.png" class="magnet" alt="magnet link" /></a> </small><br/>
            {{end}}
            <br class="clear"/>
            </div>
        {{end}}
    </div>
    {{if results}}
    <div id="footer">
        {{if pagination}}
        <div id="pagination">
            {{begin pagination}}
            {{unless selected}}<a href="?{{unless $_first}}p={{$_num}}&{{end}}q={{query}}">{{$_num}}</a>{{end}}
            {{if selected}}<b>{{$_num}}</b>{{end}}
            {{unless $_last}}, {{end}}
            {{end}}
            <br class="clear"/>
            <span class="ctrlhint">
                {{if prevlink}}<a href="{{prevlink}}">&larr;</a>{{end}}
                {{unless prevlink}}&larr;{{end}}
                ctrl
                {{if nextlink}}<a href="{{nextlink}}">&rarr;</a>{{end}}
                {{unless nextlink}}&rarr;{{end}}
            </span>
        </div>
        {{end}}
        <div id="copyright">
            Powered by Sphinx <img src="img/sphinx.gif" alt="sphinx" /><br/>
            {{time}}s
        </div>
        <br class="clear"/>
    </div>
    {{end}}
</body>
</html>