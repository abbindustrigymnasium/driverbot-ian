<script>
    let showSecondQuestion = false;
    let numberOfPlayers = 1; // Default value

    function handleButtonClick() {
        showSecondQuestion = true;
    }

    function handleSecondButtonClick() {
        // Get the number of players from the input field
        numberOfPlayers = document.getElementById('players').value;
        // Redirect to the controller page with the number of players as a parameter
        window.location.href = `/controller?players=${numberOfPlayers}`;
    }
</script>

<section id="contents">
    <span class="info">Place out the start and finish line</span>
    <div class="button" on:click={handleButtonClick}><span>Done</span></div>

    {#if showSecondQuestion}
        <span id="button2" class="info">How many players?</span>
        <div><input id="players" placeholder="1"/></div>
        <div class="button" on:click={handleSecondButtonClick}><span>Done</span></div>
    {/if}
</section>
  
  <style>
    .info {
      font-size: 30px;
    }
    #contents {
      padding-top: 20px;
      margin: auto;
      width: fit-content;
    }
    #players {
      font-size: 30px;
      border-bottom: 2px solid #c4c4c4;
      border-top: none;
      border-right: none;
      border-left: none;
      width: 1rem;
    }
    #players:focus {   
      outline: none; 
    }
    .button {
      cursor: pointer;
    }
  </style>
  