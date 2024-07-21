function submitSettings() {
  const setting1 = document.getElementById('setting1').value;
  const setting2 = document.getElementById('setting2').value;

  fetch('/api/settings', {
      method: 'POST',
      headers: {
          'Content-Type': 'application/json'
      },
      body: JSON.stringify({
          setting1: setting1,
          setting2: setting2
      })
  })
  .then(response => response.json())
  .then(data => {
      alert('Settings saved: ' + JSON.stringify(data));
  })
  .catch(error => {
      console.error('Error:', error);
  });
}