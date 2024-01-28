import { useErrorStore } from './stores/error'

function dealResponse(response) {
  if (response.code) {
    throw new Error(response.msg || '未知错误')
  }
  return response.data
}

export const http_get = (url, params) => {
  return fetch(url, {
    method: 'GET',
    headers: {
      'Content-Type': 'application/json'
    },
    params: params
  })
    .then((response) => {
      if (response.ok) {
        return response.json()
      } else {
        throw new Error('HTTP error ' + response.status)
      }
    })
    .then((response) => dealResponse(response))
    .catch((error) => {
      const { showError } = useErrorStore()
      showError(error.message)
      throw error
    })
}

export const http_post = (url, body, params) => {
  return fetch(url, {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    params: params,
    body: JSON.stringify(body)
  })
    .then((response) => {
      if (response.ok) {
        return response.json()
      } else {
        new Error('HTTP error ' + response.status)
      }
    })
    .then((response) => dealResponse(response))
    .catch((error) => {
      const { showError } = useErrorStore()
      showError(error.message)
      throw error
    })
}
