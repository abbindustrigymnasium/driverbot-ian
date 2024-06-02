<script>
	import './styles.css';
</script>

<div class="app">
	<main>
		<slot />
	</main>
</div>

<style>
	:global(body) {
		font-family: sans-serif;
		overflow: hidden
	}
</style>