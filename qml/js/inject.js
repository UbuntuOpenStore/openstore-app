//Let's remove the header from the website when we Sign In
var styles = `
    #navigation { display: none !important }
`

var styleSheet = document.createElement("style")
styleSheet.type = "text/css"
styleSheet.innerText = styles
document.head.appendChild(styleSheet)
