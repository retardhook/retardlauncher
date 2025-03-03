package main

import (
	"github.com/webview/webview_go"
	"io/ioutil"
	"log"
)

func main() {
	htmlContent, err := ioutil.ReadFile("launcher.html")
	if err != nil {
		log.Fatalf("Error reading HTML file: %v", err)
	}

	w := webview.New(false)
	defer w.Destroy()
	w.SetTitle("Minecraft Launcher")
	w.SetSize(480, 320, webview.HintNone)
	w.SetHtml(string(htmlContent))
	w.Run()
}
