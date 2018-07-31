let questionSelectionCount = 0;
$(() => {
    document.getElementById("selection-count").innerText = `0 Questions Selected`;
})();

function handleCheckboxChange(checkbox) {
    console.log(checkbox);
    console.log(document.getElementById("selection-count"));
    if (checkbox.checked === true){
        document.getElementById("selection-count").innerText = `${++questionSelectionCount}  Questions Selected`;
    }else{
        document.getElementById("selection-count").innerText = `${--questionSelectionCount}  Questions Selected`;
   }
}