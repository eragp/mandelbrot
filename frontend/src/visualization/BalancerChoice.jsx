import React, { Component } from 'react';
import BalancerPolicy from '../misc/BalancerPolicy';

export default class BalancerChoice extends Component {
  constructor(props) {
    super(props);

    this.props.balancerPolicy.setBalancer('naive');

    this.state = {
      active: this.props.balancerPolicy.getBalancer(),
      items: [
        {
          key: 'naive',
          title: 'Naive Balancer'
        },
        {
          key: 'column',
          title: 'Column Balancer'
        },
        {
          key: 'prediction',
          title: 'Prediction Balancer'
        }
      ]
    };
  }

  handleBalancerChoice(balancer) {
    this.props.balancerPolicy.setBalancer(balancer);

    this.setState(state => {
      state.active = this.props.balancerPolicy.getBalancer();
      return state;
    });
  }

  render() {
    const itemList = [];
    for (let item of this.state.items) {
      const active = item.key == this.state.active;
      const handler = item.disabled
        ? () => false
        : () => this.handleBalancerChoice(item.key);
      itemList.push(
        <BalancerItem
          key={item.key}
          title={item.title}
          active={active}
          disabled={item.disabled}
          handleClick={handler}
        />
      );
    }

    return <div className="list-group balancerChoice">{itemList}</div>;
  }
}

// BalancerChoice.propTypes = {
//   balancerPolicy: PropTypes.instanceOf(BalancerPolicy)
// };

function BalancerItem (props) {
    let classList = 'list-group-item list-group-item-action';
    if (props.active) {
      classList += ' active';
    }
    if (props.disabled) {
      classList += ' disabled';
    }
    return (
      <a href="#" className={classList} onClick={props.handleClick}>
        {props.title}
      </a>
    );
  }

// BalancerItem.propTypes = {
//   active: PropTypes.bool,
//   disabled: PropTypes.bool,
//   handleClick: PropTypes.func,
//   title: PropTypes.string.isRequired
// };

// BalancerItem.defaultProps = {
//   active: false,
//   disabled: false,
//   handleClick: () => {}
// };
