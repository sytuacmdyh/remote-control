import { computed, reactive, ref, watchEffect } from 'vue'
import { defineStore } from 'pinia'
import { http_get, http_post } from '../request'

const productActionDef = {
  电灯: [
    { name: 'turn-on', nameCN: '开灯' },
    { name: 'turn-off', nameCN: '关灯' }
  ]
}

export const useDeviceStore = defineStore('device', () => {
  const deviceName = ref('')
  const device = reactive({})
  const actionsAll = reactive([])

  watchEffect(() => {
    const { product, actions } = device
    if (!product) return
    const all = productActionDef[product] || []
    const actionMap = {}
    actions.forEach((a) => {
      actionMap[a.name] = a
    })
    actionsAll.splice(
      0,
      actionsAll.length,
      ...all.map((a) => {
        return {
          ...a,
          ...actionMap[a.name]
        }
      })
    )
  })

  const refreshDeviceDetail = () => {
    return http_get(`/api/rmt/getDeviceDetail?name=${deviceName.value}`).then((res) => {
      Object.assign(device, res)
    })
  }

  const setDevice = (name) => {
    deviceName.value = name
    return refreshDeviceDetail()
  }

  const bindAction = (device, action) => {
    return http_post('/api/rmt/bindAction', {
      device: device.name,
      action: action.name,
      addr: Number.parseInt(action.addr),
      cmd: Number.parseInt(action.cmd)
    }).then(() => refreshDeviceDetail())
  }

  const getRecentCommands = () => http_get('/api/rmt/getCmdList')

  return { device, actionsAll, setDevice, bindAction, getRecentCommands }
})
