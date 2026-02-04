console.log("Loading Highlightjs hook")

$(function() {
	hljs.configure({ useBR: false });
	$(".fragment").each((i, node) => {
		const rawText = $(node).find(".line")
			.map((_, el) => el.innerText).toArray()
			.join("\n");

		const codeEl = document.createElement("code");
		const { value: valueHtml } = hljs.highlight(rawText, { language: "cpp", ignoreIllegals: true });

		codeEl.innerHTML = valueHtml;
		codeEl.classList.add("hljs");

		const preCode = document.createElement("pre");
		preCode.appendChild(codeEl);
		preCode.classList.add("hljs");
		node.insertAdjacentElement('afterend', preCode);
	});
});
