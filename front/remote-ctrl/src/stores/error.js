// store.js
import { defineStore } from 'pinia'
import { ref } from 'vue'

const errorMessage = ref('')
const snackbar = ref(false)
const timeout = ref(3000)

function showError(msg, timeout_ms = 3000) {
  console.log('showError', msg, timeout_ms)
  errorMessage.value = msg
  snackbar.value = true
  timeout.value = timeout_ms
}
function hideError() {
  errorMessage.value = ''
  snackbar.value = false
}

export const useErrorStore = defineStore('error', () => {
  return { errorMessage, snackbar, timeout, showError, hideError }
})
